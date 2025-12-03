#include <getopt.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#include "dhcp.h"
#include "format.h"
#include "port_utils.h"
#include "server.h"
#include "utils.h"

#define MAX_IPS 5
#define BUFFER_SIZE 1000

#include <arpa/inet.h>
#include <assert.h>
#include <netdb.h>
#include <netinet/in.h>
#include <semaphore.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

// #include "utils.h"

/* Basic framework for a web server than can handle a single request, then
   shuts down. Looks for files in the current directory.

   If the file is found, returns the total length of the response (including
   the HTTP headers). If not, returns -1 after writing a 404 message to the
   socket. Also returns -1 if any errors occur along the way.
 */
ssize_t
serve_web (char *protocol)
{
  // Pass the protocol (should be a port number) to setup_server() to get the
  // server socket.
  int socketfd = setup_server (protocol);
  if (socketfd < 0)
    return -1;

  // Indicate (for debugging) that the server is running
  fprintf (stderr, "Server is started on port %s\n", protocol);

  struct sockaddr_in sa;
  memset (&sa, 0, sizeof (sa));
  socklen_t len_sa = sizeof (sa);

  uint16_t port = (uint16_t) strtol (protocol, NULL, 10);

  // setup the server to receive the message from the client
  sa.sin_family = AF_INET;
  sa.sin_port = htons (port);
  inet_pton (AF_INET, "127.0.0.1", &(sa.sin_addr));
  size_t length = MAX_DHCP_LENGTH;

  uint8_t lease_time[] = {0x0, 0x27, 0x8d, 0x0};
  uint8_t sid[] = {192, 168, 1, 0};

  uint8_t buf[length];
  //client_t clients[4];
  client_t clients[6];
  ip_t active_ips[4];
  memset(active_ips,0, sizeof(active_ips));
  memset (clients, 0, sizeof (clients));
  for(int i = 0; i < 4; i++){
    active_ips[i].ciaddr_digit = i + 1;
  }

  // index of clients array to know where to append
  // size_t index = 0;
  int cur_clients = 0;
  int num_clients = 0;
  memset (clients, 0, sizeof (clients));
  //while (num_clients < 5)
  while (cur_clients < 5)
    {
      memset (buf, 0, length);
      // receive the message from the client
      ssize_t nbytes = recvfrom (socketfd, buf, length, 0,
                                (struct sockaddr *)&sa, &len_sa);

      if (nbytes < 0)
        {
          shutdown (socketfd, SHUT_RDWR);
          close (socketfd);
          return -1;
        }

      options_t options;
      memset (&options, 0, sizeof (options_t));

      uint8_t *start = buf + sizeof (msg_t) + 4;
      uint8_t *end = buf + nbytes - 1;
      get_options (start, end, &options);

      size_t size = sizeof (msg_t);
      uint8_t *packet = calloc (size, sizeof (uint8_t));
      memcpy (packet, buf, size);

      msg_t *message = (msg_t *)packet;

      client_t client;
      
      switch (*options.type)
        {
          case DHCPDISCOVER:
            //if (num_clients >= 0 && num_clients < 4)
            if (cur_clients >= 0 && cur_clients < 4)
              {
                cur_clients++;
                *options.type = DHCPOFFER;
                memset (&client, 0, sizeof (client_t));
                client.tombstone = false;
                for (int i = 0; i < num_clients; i++)
                {
                  if (memcmp (&message->chaddr, &clients[i].chaddr, sizeof (message->chaddr)) == 0)
                    {
                      client = clients[i];
                    }
                  }
                if (client.xid == 0)
                  {
                    num_clients++;
                    client.xid = message->xid;
                    memcpy (&client.chaddr, &message->chaddr, sizeof (client.chaddr));
                    char ciaddr[12];
                    if(num_clients > 4){
                      for(int i = 0; i < 4; i++){
                        if(!active_ips[i].active){
                          snprintf (ciaddr, 12, "192.168.1.%d", active_ips[i].ciaddr_digit);
                          inet_pton (AF_INET, ciaddr, &client.ciaddr.s_addr);
                          clients[num_clients - 1] = client;
                          active_ips[i].active = true;
                          break;
                        }
                      }
                    }else{
                      if (num_clients > 0 && num_clients < 5)
                        snprintf (ciaddr, 12, "192.168.1.%d", num_clients);
                    inet_pton (AF_INET, ciaddr, &client.ciaddr.s_addr);
                    clients[num_clients - 1] = client;
                    active_ips[cur_clients].active = true;
                    }
                  }
              }
            else
              *options.type = DHCPNAK;
            break;
          case DHCPREQUEST:
            *options.type = memcmp (&options.sid->s_addr, sid, sizeof (sid)) == 0
                            ? DHCPACK : DHCPNAK;
            for (int i = 0; i < num_clients; i++)
                {
                  if (memcmp (&message->chaddr, &clients[i].chaddr, sizeof (message->chaddr)) == 0)
                    {
                      client = clients[i];
                    }
                  }
            break;
          case DHCPRELEASE:
            for (int i = 0; i < 4; i++) {
              if (memcmp(&message->chaddr, &clients[i].chaddr, sizeof(message->chaddr)) == 0) {
                clients[i].tombstone = true;
                active_ips[cur_clients].active = false;
                cur_clients--;
                break;
              }
          }
        }

      if (*options.type != DHCPRELEASE)
        {
          packet = append_cookie (packet, &size);
          packet = append_option (packet, &size, DHCP_opt_msgtype, 1, options.type);
          packet = append_option (packet, &size, DHCP_opt_sid, 4, sid);
          if (*options.type != DHCPNAK)
            packet = append_option (packet, &size, DHCP_opt_lease, 4, lease_time);
          packet[size++] = 0xff;
    
          message->op = BOOTREPLY;
          // memset (&message->ciaddr, 0, sizeof (message->ciaddr));
          char yiaddr[12];

          if (*options.type != DHCPNAK)
          {
            if (message->xid == 0)
              {
                snprintf (yiaddr, 12, "192.168.1.1");
                inet_pton (AF_INET, yiaddr, &message->yiaddr.s_addr);
              }
            else
              memcpy (&message->yiaddr.s_addr, &client.ciaddr.s_addr, sizeof (message->yiaddr.s_addr));
          }
    
          // if (message->xid == 0)
          //   snprintf (yiaddr, 12, "192.168.1.1");
          // else
          //   snprintf (yiaddr, 12, "192.168.1.%ld", num_clients);
          // if (*options.type != DHCPNAK)
          //   inet_pton (AF_INET, yiaddr, &message->yiaddr.s_addr);
          
          printf ("SERVER SENT\n");
          switch (*options.type)
            {
            case DHCPOFFER:
              printf ("DHCPOFFER\n");
              printf ("---------");
              break;
            case DHCPACK:
              printf ("DHCPACK\n");
              printf ("-------");
              break;
            case DHCPRELEASE:
              printf ("DHCPRELEASE\n");
              printf ("-----------");
              break;
            case DHCPNAK:
              printf ("DHCPNAK\n");
              printf ("-------");
            }
          printf ("------------\n");
          dump_packet (packet, size);
    
          // send back to client and shut down the server
          
          sendto (socketfd, message, size, 0, (struct sockaddr *)&sa, len_sa);
        }
      free (packet);
    }
  
  shutdown (socketfd, SHUT_RDWR);
  close (socketfd);

  return EXIT_SUCCESS;
}