// Notes:
// IP RFC:   https://datatracker.ietf.org/doc/html/rfc791
// ICMP RFC: https://datatracker.ietf.org/doc/html/rfc792

#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include "internal_utils.h"
#include "dynamic_array.h"
#include "argparse.h"
#include "limits.h"

#ifndef PING_HEADER
#define PING_HEADER
#if __linux__
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <linux/icmp.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <time.h>

static int send_icmp_echo_request(int icmp_socket, uCharArray *payload, struct addrinfo *dst_addrinfo, unsigned short n);
static int receive_echo_reply(int icmp_socket, uCharArray *payload, struct addrinfo *dst_addrinfo);
static int linux_ping_cycle(char *dst, uCharArray *payload, unsigned short n);

#elif _WIN32
#endif // __linux__ || _WIN32
#define ICMP_PROTO_NUMBER 1
#define IPV4_HEADER_MAX_SIZE 60 // IHL: 4 bits. Internet Header Length is the length of the internet header in 32 bit words,
                                // and thus points to the beginning of the data.

static unsigned short csum(unsigned short *buf, int nwords);
static void word_pad(unsigned char *buff, size_t payload_size, size_t *required_size, char pad_byte);
static int ping_implementation(pArglist arg_list);
// dst     - IPv4 address or domain name
// payload - Optional data to send with echo request
// n       - Number of requests to send, set 0 to have infinite
int ping_cycle(char *dst, uCharArray *payload, unsigned short n);
int ping_main(int argc, char **argv);

#ifdef PING_HEADER_IMPLEMENTATION

int ping_main(int argc, char **argv)
{
  Arglist arg_list = {.footer_msg = "ping [options] destination"};
  push_argument(&arg_list, (Argument){.key = "-h", .flag = IS_FLAG, .help_msg = "Prints this help message."});
  push_argument(&arg_list, (Argument){.key = "-n", .flag = ARG_OPTIONAL, .help_msg = "Times to ping. By default ping in infinite loop."});
  parse_arguments(argc, argv, &arg_list);
  if (is_flag_set(&arg_list, "-h") || argc == 1)
  {
    print_default_help(&arg_list);
    exit(0);
  }
  else
    ping_implementation(&arg_list);
  return 0;
}

int ping_implementation(pArglist arg_list)
{
  unsigned long long times_to_ping = 1;
  size_t pos = 0;
  char *dst = get_next_positional_value(arg_list, &pos);

  if (!dst)
    report_error_and_exit("destination host is not provided\n");

  if (is_value_set(arg_list, "-n"))
  {
    times_to_ping = strtoull(get_value_by_key(arg_list, "-n"), NULL, 10);
    if (times_to_ping == 0)
      report_error_and_exit("wrong value specified for -n '%s'\n", get_value_by_key(arg_list, "-n"));
    ping_cycle(dst, NULL, times_to_ping);
  }
  else
  {
    ping_cycle(dst, NULL, 0);
  }

  return 0;
}

int ping_cycle(char *dst, uCharArray *payload, unsigned short n)
{
#if __linux__
  return linux_ping_cycle(dst, payload, n);
#elif _WIN32
  report_error_and_exit("Not implemented");
#else
  report_error_and_exit("Platform not supported");
#endif // Platform selection
  return 1;
}

static unsigned short csum(unsigned short *buf, int nwords)
{
  unsigned long sum;
  for (sum = 0; nwords > 0; nwords--)
    sum += *buf++;
  sum = (sum >> 16) + (sum & 0xffff);
  sum += (sum >> 16);
  return (unsigned short)(~sum);
}

static void word_pad(unsigned char *buff, size_t payload_size, size_t *required_size, char pad_byte)
{
  if (*required_size == 0)
  {
    *required_size = (payload_size % 2) + payload_size;
  }
  else
  {
    for (size_t i = payload_size; i < *required_size; ++i)
      buff[i] = pad_byte;
  }
}

#if __linux__

static int linux_ping_cycle(char *dst, uCharArray *payload, unsigned short n)
{
  struct addrinfo in_addr = {0};
  struct addrinfo *dst_addrinfo;
  int icmp_socket;
  char resolved_addr_str[INET_ADDRSTRLEN]; // For resolved dst IPv4 string
  struct timespec start, end;

  in_addr.ai_family = AF_INET; // ICMP only IPv4
  in_addr.ai_socktype = SOCK_RAW;
  in_addr.ai_protocol = ICMP_PROTO_NUMBER;
  if (getaddrinfo(dst, NULL, &in_addr, &dst_addrinfo))
  {
    perror("Cannot resolve host");
    exit(1);
  }
  inet_ntop(AF_INET, &((struct sockaddr_in *)dst_addrinfo->ai_addr)->sin_addr.s_addr, resolved_addr_str, sizeof(resolved_addr_str));

  icmp_socket = socket(dst_addrinfo->ai_family, dst_addrinfo->ai_socktype, dst_addrinfo->ai_protocol);
  if (icmp_socket == -1)
  {
    perror("Cannot create raw ICMP socket");
    exit(1);
  }

  if (n)
  {
    for (size_t i = 0; i < n; ++i)
    {
      send_icmp_echo_request(icmp_socket, payload, dst_addrinfo, i);
      printf("Sent request to %s(%s) icmp_seq: %zu\n", dst, resolved_addr_str, i);
      clock_gettime(CLOCK_MONOTONIC, &start);
      while (i != receive_echo_reply(icmp_socket, payload, dst_addrinfo))
        ;
      clock_gettime(CLOCK_MONOTONIC, &end);
      long ms = (end.tv_sec - start.tv_sec) * 1000 + (end.tv_nsec - start.tv_nsec) / 1000000;
      printf("Received reply in %ld ms for icmp_seq: %zu\n", ms, i);
    }
  }
  else
  {
    for (size_t i = 0; i < USHRT_MAX; ++i)
    {
      send_icmp_echo_request(icmp_socket, payload, dst_addrinfo, i);
      printf("Sent request to %s(%s) icmp_seq: %zu\n", dst, resolved_addr_str, i);
      clock_gettime(CLOCK_MONOTONIC, &start);
      while (i != receive_echo_reply(icmp_socket, payload, dst_addrinfo))
        ;
      clock_gettime(CLOCK_MONOTONIC, &end);
      long ms = (end.tv_sec - start.tv_sec) * 1000 + (end.tv_nsec - start.tv_nsec) / 1000000;
      printf("Received reply in %ld ms for icmp_seq: %zu\n", ms, i);
      if (i == USHRT_MAX - 1)
        i = 0;
    }
  }

  freeaddrinfo(dst_addrinfo);
  return 0;
}

static int send_icmp_echo_request(int icmp_socket, uCharArray *payload, struct addrinfo *dst_addrinfo, unsigned short icmp_sequence)
{
  size_t icmp_packet_size, padded_payload_size;
  unsigned char *icmp_packet = 0;
  struct icmphdr icmp_header = {0};
  icmp_packet_size = padded_payload_size = 0;

  if (payload)
    word_pad(NULL, payload->count, &padded_payload_size, '\0'); // Find final packet size when payload will be padded
  icmp_packet_size = sizeof(icmp_header) + padded_payload_size; // ICMP Echo Message header + padded payload
  icmp_packet = malloc(icmp_packet_size);
  if (!icmp_packet)
  {
    perror("Cannot allocate memory for buffer\n");
    exit(1);
  }

  icmp_header.type = ICMP_ECHO;
  icmp_header.code = 0;
  icmp_header.checksum = 0; // For computing the checksum, the checksum field should be zero.
  icmp_header.un.echo.id = htons(getpid() & 0xffff);
  icmp_header.un.echo.sequence = htons(icmp_sequence);

  memcpy(icmp_packet, &icmp_header, sizeof(icmp_header));
  if (payload)
  {
    memcpy((unsigned char *)icmp_packet + sizeof(icmp_header), payload->array, payload->count);
    word_pad((unsigned char *)icmp_packet + sizeof(icmp_header), payload->count, &padded_payload_size, '\0');
  }

  icmp_header.checksum = csum((unsigned short *)icmp_packet, icmp_packet_size / 2);
  memcpy(icmp_packet, &icmp_header, sizeof(icmp_header));

  if (sendto(icmp_socket, icmp_packet, icmp_packet_size, 0, dst_addrinfo->ai_addr, dst_addrinfo->ai_addrlen) == -1)
  {
    perror("Error sending ICMP");
    exit(1);
  }
  free(icmp_packet);
  return 0;
}

static int receive_echo_reply(int icmp_socket, uCharArray *payload, struct addrinfo *dst_addrinfo)
{
  unsigned char *data = NULL;
  size_t expected_packet_size = 0, padded_payload_size = 0;

  ssize_t bytes_read = 0;
  struct sockaddr recv_addr = {0};
  struct sockaddr_in *recv_addr_in = NULL;
  socklen_t recv_addr_len = sizeof(struct sockaddr);
  unsigned char recv_ipv4_hdr_size = 0;

  struct icmphdr icmp_hdr = {0};

  if (payload)
    word_pad(NULL, payload->count, &padded_payload_size, '\0');
  expected_packet_size = IPV4_HEADER_MAX_SIZE + sizeof(struct icmphdr) + padded_payload_size;
  data = malloc(expected_packet_size);
  if (!data)
  {
    perror("Cannot allocate memory for buffer\n");
    exit(1);
  }
  memset(data, '\0', expected_packet_size);

  // Waiting to get ICMP echo reply from dst with correct Identifier that correspond to this process
  // Sequence Number "icmp_sequence" in theory can come out of order, so it will be returned to caller to decide what to do
  while (1)
  {
    bytes_read = recvfrom(icmp_socket, data, expected_packet_size, 0, &recv_addr, &recv_addr_len);
    if (bytes_read == -1)
    {
      perror("Failed to receive data");
      exit(1);
    }
    recv_ipv4_hdr_size = (*data & 0x0f) * 4; // [Version: 4 bits][IHL:  4 bits], IHL is the length of the internet header in 32 bit words

    // Check if reply came from expected destination address
    recv_addr_in = (struct sockaddr_in *)&recv_addr;
    if (!(recv_addr_in->sin_addr.s_addr == ((struct sockaddr_in *)dst_addrinfo->ai_addr)->sin_addr.s_addr))
      continue;

    // Check if request was send by this process
    if (!((recv_ipv4_hdr_size + sizeof(icmp_hdr)) <= bytes_read))
      continue;
    memcpy(&icmp_hdr, data + recv_ipv4_hdr_size, sizeof(icmp_hdr));
    if (!(ntohs(icmp_hdr.un.echo.id) == (getpid() & 0xffff)))
      continue;

    size_t headers_size = recv_ipv4_hdr_size + sizeof(icmp_hdr);
    if (payload && ((bytes_read - headers_size) != padded_payload_size || memcmp(data + headers_size, payload->array, payload->count)))
      printf("Waring received ICMP echo reply with Sequence Number %d has invalid data payload.\n", ntohs(icmp_hdr.un.echo.sequence));
    free(data);
    return ntohs(icmp_hdr.un.echo.sequence);
  }
  return -1;
}

#elif _WIN32
#else

#endif // __linux__

#endif // PING_HEADER_IMPLEMENTATION

#endif // PING_HEADER