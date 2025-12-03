#include <fcntl.h>
#include <inttypes.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include "dhcp.h"

msg_t
fill_msg (char *file)
{
  int fd = open (file, O_RDONLY);
  size_t length = sizeof (msg_t);
  uint8_t buffer[length + 1];
  memset (buffer, 0, length + 1);

  read (fd, buffer, length);

  msg_t *message = (msg_t *) buffer;

  message->xid = htonl (message->xid);
  message->secs = htons(message->secs);

  return *message;
}

/* Helper function for debugging purposes only. Given an array of binary
   data, print each byte in 2-digit hex format similar to calling hexdump on
   a file. For instance, if you read the contents of tests/data/bootp-1-eth
   into an array, the first line of output calling this function will look
   like:

     01 01 06 00 00 00 00 2a . 00 00 00 00 7f 00 00 01
 */
void
dump_packet (uint8_t *ptr, size_t size)
{
  size_t index = 0;
  while (index < size)
    {
      fprintf (stderr, " %02" PRIx8, ptr[index++]);
      if (index % 32 == 0)
        fprintf (stderr, "\n");
      else if (index % 16 == 0)
        fprintf (stderr, "  ");
      else if (index % 8 == 0)
        fprintf (stderr, " .");
    }
  if (index % 32 != 0)
    fprintf (stderr, "\n");
  fprintf (stderr, "\n");
}
