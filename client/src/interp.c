#include <stdlib.h>
#include <string.h>

#include "dhcp.h"
#include "format.h"

int
interp (char *file)
{
  msg_t message = fill_msg (file);
  // dump_packet((uint8_t *)&message, sizeof(msg_t) + 128); 
  dump_msg (&message, sizeof (msg_t));
  return EXIT_SUCCESS;
}
