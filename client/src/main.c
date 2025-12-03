#include <arpa/inet.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include <netdb.h>
#include <unistd.h>

#include "interp.h"
#include "client.h"
#include "dhcp.h"
#include "format.h"
#include "port_utils.h"

int cmdline (int, char **, msg_t *, int *);

int
main (int argc, char **argv)
{
  if (argc == 2)
    {
      return interp (argv[1]);
      // return EXIT_SUCCESS;
    }
  
  msg_t message;
  memset (&message, 0, sizeof (message));
  message.op = 1;
  message.xid = 42;
  message.htype = ETH;
  message.hlen = ETH_LEN;
  for (int i = 0; i < message.hlen; i++)
    message.chaddr[i] = i + 1;

  int i = 0;
  // magic cookie
  message.options[i++] = 0x63; // options[0]
  message.options[i++] = 0x82; // options[1]
  message.options[i++] = 0x53; // options[2]
  message.options[i++] = 0x63; // options[3]
  // message type
  message.options[i++] = 0x35; // options[4]
  message.options[i++] = 0x01; // options[5]
  message.options[i++] = DHCPDISCOVER; // options[6]
  // requested IP
  message.options[i++] = 0x32; // options[7]
  message.options[i++] = 0x04; // options[8]
  
  uint8_t request[4] = {127, 0, 0, 2};
  for (int j = 0; j < 4; j++)
    message.options[i++] = request[j]; // options[9-12]

  // server IP
  message.options[i++] = 0x36; // options[13]
  message.options[i++] = 0x04; // options[14]
  uint8_t server[4] = {127, 0, 0, 1};
  for (int j = 0; j < 4; j++)
    message.options[i++] = server[j]; // options[15-18]

  int p_flag = 0;
  // finish
  message.options[i] = 0xff; // options[19]
  // fill in message from commandline arguments
  int packet_len;
  if (argc > 2)
    p_flag = cmdline (argc, argv, &message, &packet_len);

  if (p_flag == 1)
    dump_msg (&message, sizeof (msg_t));
  else
    {
      struct sockaddr_in sa;
      socklen_t len_sa = sizeof (sa);

      char *port_char = get_port();
      uint16_t port = (uint16_t) strtol (port_char, NULL,10);

      memset (&sa, 0, sizeof (sa));
      sa.sin_family = AF_INET;
      sa.sin_port = htons (port);
      inet_pton (AF_INET, "127.0.0.1", &(sa.sin_addr));

      int fd = socket (AF_INET, SOCK_DGRAM, 0);

      printf ("+++++++++++++++\n");
      printf ("CLIENT SENDING:\n");
      printf ("+++++++++++++++\n\n");
      dump_msg (&message, sizeof (message));
//recvfrom (fd, response, 512, 0, (struct sockaddr *)&addr, &length);
      size_t length = sizeof (msg_t) - sizeof (message.options) + (packet_len - 1);
      sendto (fd, &message, length, 0, (struct sockaddr *) &sa, len_sa);
      if(message.xid != 0){
        uint8_t buffer[sizeof (msg_t)];
        memset (buffer, 0, sizeof (msg_t));
        recvfrom (fd, buffer, sizeof (msg_t), 0, (struct sockaddr *) &sa, &len_sa);

        msg_t *received_message = (msg_t *) buffer;
        printf ("\n++++++++++++++++\n");
        printf ("CLIENT RECEIVED:\n");
        printf ("++++++++++++++++\n\n");
        dump_msg (received_message, sizeof (*received_message));

        received_message->op = BOOTREQUEST;
        memcpy (&received_message->options[9], &received_message->yiaddr.s_addr, 4);
        received_message->yiaddr.s_addr = 0;
        received_message->options[6] = DHCPREQUEST;
        // memset (&received_message->options[15], 0, 4);

        // memset (received_message->options[])

        printf ("\n+++++++++++++++\n");
        printf ("CLIENT SENDING:\n");
        printf ("+++++++++++++++\n\n");
        dump_msg (received_message, sizeof (msg_t));
        length = sizeof (msg_t) - sizeof (received_message->options) + OTHER_PACKLEN + 6;
        sendto (fd, received_message, length, 0, (struct sockaddr *) &sa, len_sa);

        memset (buffer, 0, sizeof (msg_t));
        recvfrom(fd, buffer, sizeof (msg_t), 0, (struct sockaddr *) &sa, &len_sa);
        received_message = (msg_t *) buffer;
        printf ("\n++++++++++++++++\n");
        printf ("CLIENT RECEIVED:\n");
        printf ("++++++++++++++++\n\n");
        dump_msg (received_message, sizeof (*received_message));
        printf ("\n");
      }
      close (fd);
    }
  // return client (argc, argv);
  return EXIT_SUCCESS;
}

int
cmdline (int argc, char **argv, msg_t *message, int *packet_len)
{
  *packet_len = DISCOVER_PACKLEN;
  int option;
  int p_flag = 1;
  while ((option = getopt (argc, argv, "x:t:c:m:s:r:p")) != -1)
    {
      switch (option)
        {
        case 'x':
          message->xid = strtol (optarg, NULL, 10);
          break;
        case 't':
          message->htype = strtoul (optarg, NULL, 10);
          switch (message->htype)
            {
            case IEEE802:
              message->hlen = IEEE802_LEN;
              break;
            case ARCNET:
              message->hlen = ARCNET_LEN;
              break;
            case FRAME_RELAY:
              message->hlen = FRAME_LEN;
              break;
            case FIBRE:
              message->hlen = FIBRE_LEN;
              break;
            }
          break;
        case 'c':
          memset (message->chaddr, 0, sizeof (message->chaddr));
          for (int i = 0, j = 0; i < strlen (optarg); i += 2, j++)
            {
              char byte[3] = {optarg[i], optarg[i+1], '\0'};
              int value = strtoul (byte, NULL, 16);
              message->chaddr[j] = value;
            }
          break;
        case 'm':;
          uint8_t m_type = strtoul (optarg, NULL, 10);
          message->options[6] = m_type;
          switch (m_type)
            {
            case DHCPDISCOVER:
              *packet_len = DISCOVER_PACKLEN;
              break;
            default:
              *packet_len = OTHER_PACKLEN;
              break;
            }
          break;
        case 's':;
          char *server = strtok (strdup (optarg), ".");
          int i = 15;
          for (int j = 0; j < 4; j++)
            {
              message->options[i++] = strtol (server, NULL, 10);
              server = strtok (NULL, ".");
            }
          break;
        case 'r':;
          server = strtok (strdup (optarg), ".");
          i = 9;
          for (int j = 0; j < 4; j++)
            {
              message->options[i++] = strtol (server, NULL, 10);
              server = strtok (NULL, ".");
            }
          break;
        case 'p':
          p_flag = 0;
          break;
        }
    }
  return p_flag;
}