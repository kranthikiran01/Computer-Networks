#include <stdio.h>
#include <stdlib.h>
#include <libnet.h>
#include <stdint.h>

int main() {

  libnet_t *l;  /* libnet context */
  char str[10];
  char errbuf[LIBNET_ERRBUF_SIZE], ip_addr_str[16];
  u_int32_t src_ip_addr,dst_ip_addr;
   libnet_ptag_t udp=0,ip=0;
   u_short src_prt, dst_prt;
  u_int16_t id, seq;
  char payload[] = "libnet :D";
  int bytes_written;

  l = libnet_init(LIBNET_RAW4, NULL, errbuf);
  if ( l == NULL ) {
    fprintf(stderr, "libnet_init() failed: %s\n", errbuf);
    exit(EXIT_FAILURE);
  }

  /* Generating a random id */

  libnet_seed_prand (l);
  id = (u_int16_t)libnet_get_prand(LIBNET_PR16);

  /* Getting destination IP address */

  printf("source IP address: ");
  scanf("%15s",ip_addr_str);

 src_ip_addr = libnet_name2addr4(l, ip_addr_str,\
                  LIBNET_DONT_RESOLVE);

if ( src_ip_addr == -1 ) {
    fprintf(stderr, "Error converting IP address.\n");
    libnet_destroy(l);
    exit(EXIT_FAILURE);
  }


  printf("Destination IP address: ");
  scanf("%15s",ip_addr_str);

dst_ip_addr = libnet_name2addr4(l, ip_addr_str,\
                  LIBNET_DONT_RESOLVE);

if ( dst_ip_addr == -1 ) {
    fprintf(stderr, "Error converting IP address.\n");
    libnet_destroy(l);
    exit(EXIT_FAILURE);
  }


   printf("source port: ");
  scanf("%s",str);

 src_prt=(u_short)atoi(str);

 printf("Destination port: ");
  scanf("%s",str);

  dst_prt=(u_short)atoi(str);

  

  if ( src_ip_addr == -1 ) {
    fprintf(stderr, "Error converting IP address.\n");
    libnet_destroy(l);
    exit(EXIT_FAILURE);
  }



/* Building UDP header */

 udp = libnet_build_udp(
            src_prt,                                /* source port */
            dst_prt,                            /* destination port */
            LIBNET_UDP_H + sizeof(payload),               /* packet length */
            0,                                      /* checksum */
            (uint8_t*)payload,                     /* payload */
            sizeof(payload),                              /* payload size */
            l,                                      /* libnet handle */
            udp);                                   /* libnet id */
        if (udp == -1)
        {
            fprintf(stderr, "Can't build UDP header: %s\n", libnet_geterror(l));
              exit(EXIT_FAILURE);
        }



  /* Building IP header */

  ip = libnet_build_ipv4(
                LIBNET_IPV4_H + 20 + sizeof(payload)+ LIBNET_UDP_H, /* length */
                0,                                          /* TOS */
                242,                                        /* IP ID */
                0,                                          /* IP Frag */
                64,                                         /* TTL */
                IPPROTO_UDP,                                /* protocol */
                0,                                          /* checksum */
                src_ip_addr,
                dst_ip_addr,
                NULL,                                       /* payload */
                0,                                          /* payload size */
                l,                                          /* libnet handle */
               ip);                                         /* libnet id */
            if (ip == -1)
            {
                fprintf(stderr, "Can't build IP header: %s\n", libnet_geterror(l));
                exit(EXIT_FAILURE);
            }

  /* Writing packet */

  bytes_written = libnet_write(l);
  if ( bytes_written != -1 )
    printf("%d bytes written.\n", bytes_written);
  else
    fprintf(stderr, "Error writing packet: %s\n",\
        libnet_geterror(l));

  libnet_destroy(l);
  return 0;
}
