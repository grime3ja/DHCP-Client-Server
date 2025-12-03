#include <arpa/inet.h>
#include <getopt.h>
#include <unistd.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "dhcp.h"
#include "format.h"
#include "port_utils.h"
#include "server.h"

static bool get_args (int, char **);

bool debug = false;

int
main (int argc, char **argv)
{
  if (debug)
    fprintf (stderr, "Shutting down\n");
  
  char *protocol = get_port ();
  serve_web (protocol);
  return EXIT_SUCCESS;
}

static bool
get_args (int argc, char **argv)
{
  int ch = 0;
  while ((ch = getopt (argc, argv, "dh")) != -1)
    {
      switch (ch)
        {
        case 'd':
          debug = true;
          break;
        default:
          return false;
        }
    }
  return true;
}
