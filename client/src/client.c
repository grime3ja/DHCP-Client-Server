#include <stdlib.h>

#include "dhcp.h"
#include "format.h"
#include "port_utils.h"

static bool get_args (int, char **);

int
client (int argc, char **argv)
{
  get_args (argc, argv);
  return EXIT_SUCCESS;
}
static bool
get_args (int argc, char **argv)
{
  return true;
}
