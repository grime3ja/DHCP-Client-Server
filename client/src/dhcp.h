#ifndef __cs361_dhcp_h__
#define __cs361_dhcp_h__

#include <netdb.h>
#include <stdbool.h>
#include <stdint.h>

// For reference, see:
// https://www.iana.org/assignments/bootp-dhcp-parameters/bootp-dhcp-parameters.xhtml
// https://www.iana.org/assignments/arp-parameters/arp-parameters.xhtml

// Rules for MUST and MUST NOT options, as well as values of BOOTP fields:
// https://www.rfc-editor.org/rfc/rfc2131

// Option interpretations:
// https://www.rfc-editor.org/rfc/rfc2132

#define MAX_DHCP_LENGTH 576

// Possible BOOTP message op codes:
#define BOOTREQUEST 1
#define BOOTREPLY 2

// Hardware types (htype) per ARP specifications:
#define ETH 1
#define IEEE802 6
#define ARCNET 7
#define FRAME_RELAY 15
#define FIBRE 18
#define ASYNC 19

// Hardware address lengths (hlen) per ARP specifications:
#define ETH_LEN 6
#define IEEE802_LEN 6
#define ARCNET_LEN 1
#define FRAME_LEN 2
#define FIBRE_LEN 3

// For DHCP messages, options must begin with this value:
#define MAGIC_COOKIE 0x63825363

// DHCP Message types
// If client detects offered address in use, MUST send DHCP Decline
// If server detects requested address in use, SHOULD send DHCP NAK
// Client can release its own IP address with DHCP Release
#define DHCPDISCOVER 1
#define DHCPOFFER 2
#define DHCPREQUEST 3
#define DHCPDECLINE 4
#define DHCPACK 5
#define DHCPNAK 6
#define DHCPRELEASE 7

#define DISCOVER_PACKLEN 8
#define OTHER_PACKLEN 20
// #define DISCOVER_PACKLEN

// BOOTP message type struct. This has an exact size. Add other fields to
// match the structure as defined in RFC 1542 and RFC 2131. This struct
// should NOT include the BOOTP vend field or the DHCP options field.
// (DHCP options replaced BOOTP vend, but does not have a fixed size and
// cannot be declared in a fixed-size struct.)
typedef struct {
  uint8_t op;
  uint8_t htype;
  uint8_t hlen;
  uint8_t hops;
  uint32_t xid;
  uint16_t secs;
  uint16_t flags;
  struct in_addr ciaddr;
  struct in_addr yiaddr;
  struct in_addr siaddr;
  struct in_addr giaddr;
  uint8_t chaddr[16];
  char sname[64];
  char file[128];
  uint8_t options[312];
} msg_t;

// Utility function for printing the raw bytes of a packet:
void dump_packet (uint8_t *, size_t);

msg_t fill_msg (char *);

#endif
