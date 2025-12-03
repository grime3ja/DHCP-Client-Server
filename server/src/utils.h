#ifndef __utils_h__
#define __utils_h__

#define BUFFER_SIZE 1000

struct sockaddr_in build_address (const uint32_t, const char *);
char *build_response (char *);
int get_connection (int);
int setup_server (const char *);

#endif
