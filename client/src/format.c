#include <arpa/inet.h>
#include <stdbool.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdlib.h>

#include "dhcp.h"
#include "format.h"

char *sec_conv (int);
int dhcp_offer (msg_t *, int, int, size_t, char **, char **, char **);

char* sec_conv(int secs){
  char* buffer = calloc(30,sizeof(char));
  int days = (secs / 86400);
  secs %= 86400;
  int hours = (secs / 3600);
  secs %= 3600;
  int minutes = (secs / 60);
  secs %= 60;
  snprintf(buffer, 30, "%d Days, %d:%02d:%02d\n", days, hours, minutes, secs);
  char *dup = strdup (buffer);
  free (buffer);
  return dup;
}

void
dump_msg (msg_t *msg, size_t size)
{
  /*uint16_t msg_days;
  uint16_t msg_hours;
  uint16_t msg_minutes;*/
  printf ("------------------------------------------------------\n");
  printf ("BOOTP Options\n");
  printf ("------------------------------------------------------\n");
  printf ("Op Code (op) = %d [%s]\n", msg->op, msg->op == 1 ? "BOOTREQUEST" : "BOOTREPLY");
  printf ("Hardware Type (htype) = %d ", msg->htype);
  switch (msg->htype)
    {
    case ETH:
      printf ("[Ethernet (10Mb)]\n");
      break;
    case IEEE802:
      printf ("[IEEE 802 Networks]\n");
      break;
    case ARCNET:
      printf ("[ARCNET]\n");
      break;
    case FRAME_RELAY:
      printf ("[Frame Relay]\n");
      break;
    case FIBRE:
      printf ("[Fibre Channel]\n");
      break;
    case ASYNC:
      printf ("[Asynchronous Transmission Mode (ATM)]\n");
      break;
    }
  /*uint16_t seconds = msg->secs;
  msg_days = (msg->secs / 86400);
  seconds %= 86400;
  msg_hours = (seconds / 3600);
  seconds %= 3600;
  msg_minutes = (seconds/60);
  seconds %= 60;*/
  printf ("Hardware Address Length (hlen) = %d\n", msg->hlen);
  printf ("Hops (hops) = %d\n", msg->hops);
  printf ("Transaction ID (xid) = %d (0x%x)\n", msg->xid, msg->xid);
  printf( "Seconds (secs) = %s",sec_conv(msg->secs));
  printf("Flags (flags) = %d\n", msg->flags);
  printf("Client IP Address (ciaddr) = %s\n", inet_ntoa(msg->ciaddr));
  printf("Your IP Address (yiaddr) = %s\n", inet_ntoa(msg->yiaddr));
  printf("Server IP Address (siaddr) = %s\n", inet_ntoa(msg->siaddr));
  printf("Relay IP Address (giaddr) = %s\n", inet_ntoa(msg->giaddr));
  printf("Client Ethernet Address (chaddr) = ");
  //test
  for (int i = 0; i < msg->hlen; i++)
    printf ("%02x", msg->chaddr[i]);

  printf("\n");

  int *magic = calloc (4, sizeof (int));
  memcpy (magic, msg->options, 4);
  *magic = htonl (*magic);
  if (*magic == MAGIC_COOKIE)
    {
      printf ("------------------------------------------------------\n");
      printf ("DHCP Options\n");
      printf ("------------------------------------------------------\n");
      printf ("Magic Cookie = [OK]\n");
      printf ("Message Type = ");
      int i = 6;
      char *request = calloc (25, sizeof (char));
      char *lease_time = calloc (50, sizeof (char));
      char *identifier = calloc (35, sizeof (char));
      switch (msg->options[i])
        {
        case DHCPDISCOVER:
          printf ("DHCP Discover\n");
          break;
        case DHCPOFFER:
          printf ("DHCP Offer\n");
          i += 2;
          i++;
          while (msg->options[i] != 0xff && i != 0xff)
            {
              size_t length = msg->options[i-1];
              if (msg->options[i-2] == 51)
              {
                int sec;
                memcpy (&sec, &msg->options[i], 4);
                sec = htonl (sec);
                i -= 2;
                i = dhcp_offer (msg, i, sec, length, &request, &lease_time, &identifier);
              }
            else
              {
                i -= 2;
                i = dhcp_offer (msg, i, 0, length, &request, &lease_time, &identifier);
              }
            i += i == 0xff ? 0 : 2;
          }
          // printf ("%s\n", buffer);
          if (strlen (request) > 0)
            printf ("%s", request);
          if (strlen (lease_time) > 0)
            printf ("%s", lease_time);
          if (strlen (identifier) > 0)
            printf ("%s", identifier);
          break;
        case DHCPREQUEST:
          printf ("DHCP Request\n");
          char *request = calloc (32, sizeof (char));
          i += 3;
          // get the requested IP address from memory
          inet_ntop (AF_INET, &msg->options[i], request, 32);
          printf ("Request = %s\n", request);
          free (request);
          i += 6;
          char *sid = calloc (32, sizeof (char));
          // get the server identifier from memory
          inet_ntop (AF_INET, &msg->options[i], sid, 32);
          printf ("Server Identifier = %s\n", sid);
          free (sid);
          i += 3;
          break;
        case DHCPDECLINE:
          printf ("DHCP Decline\n");
          request = calloc (32, sizeof (char));
          i += 3;
          inet_ntop (AF_INET, &msg->options[i], request, 32);
          printf ("Request = %s\n", request);
          free (request);
          i += 6;
          sid = calloc (32, sizeof (char));
          inet_ntop (AF_INET, &msg->options[i], sid, 32);
          printf ("Server Identifier = %s\n", sid);
          free (sid);
          i += 3;
          break;
        case DHCPACK:
          printf ("DHCP ACK\n");
          i += 2;
          i++;
          while (msg->options[i] != 0xff && i != 0xff)
            {
              size_t length = msg->options[i-1];
              if (msg->options[i-2] == 51)
              {
                int sec;
                memcpy (&sec, &msg->options[i], 4);
                sec = htonl (sec);
                i -= 2;
                i = dhcp_offer (msg, i, sec, length, &request, &lease_time, &identifier);
              }
            else
              {
                i -= 2;
                i = dhcp_offer (msg, i, 0, length, &request, &lease_time, &identifier);
              }
            i += i == 0xff ? 0 : 2;
          }
          if (strlen (lease_time) > 0)
            printf ("%s", lease_time);
          if (strlen (identifier) > 0)
            printf ("%s", identifier);
          break;
        case DHCPNAK:
          printf ("DHCP NAK\n");
          i += 3;
          sid = calloc (32, sizeof (char));
          inet_ntop (AF_INET, &msg->options[i], sid, 32);
          printf ("Server Identifier = %s\n", sid);
          free (sid);
          i += 3;
          break;
        case DHCPRELEASE:
          printf ("DHCP Release\n");
          // i += 3;
          while (msg->options[i] != 54)
            i++;
          i += 2;
          sid = calloc (32, sizeof (char));
          inet_ntop (AF_INET, &msg->options[i], sid, 32);
          printf ("Server Identifier = %s\n", sid);
          free (sid);
          i += 3;
          break;
        }
      i++;
      free (request);
      free (identifier);
      free (lease_time);
    }
    free (magic);
}



int dhcp_offer(msg_t *msg, int i, int seconds, size_t length, char **request, char **lease, char **identifier)
{
  switch (msg->options[i])
    {
    case 50:;
      i += 2;
      char *sid = calloc (32, sizeof (char));
      // get the string version of the IP address from memory
      inet_ntop (AF_INET, &msg->options[i], sid, 32);
      // printf ("Request = %s\n", sid);
      snprintf (*request, 25, "Request = %s\n", sid);
      free (sid);
      i += 4;
      break;
    case 51:;
      // length of number of seconds
      i++;
      length = msg->options[i];
      i++;
      char *lease_time = sec_conv (seconds);
      // printf ("IP Address Lease Time = %s", lease_time);
      snprintf (*lease, 50, "IP Address Lease Time = %s", lease_time);
      i += length;
      break;
    case 54:
      i += 2;
      sid = calloc (32, sizeof (char));
      // get the string version of the IP address from memory
      inet_ntop (AF_INET, &msg->options[i], sid, 32);
      // printf ("Server Identifier = %s\n", sid);
      snprintf (*identifier, 35, "Server Identifier = %s\n", sid);
      free (sid);
      i += 4;
      break;
    case 255:
      return 0xff;
    }
  return i;
}