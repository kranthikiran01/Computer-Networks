/*
    Packet sniffer using libpcap library
*/
#include<pcap.h>
#include<stdio.h>
#include<stdlib.h> // for exit()
#include<string.h> //for memset
 
#include<sys/socket.h>
#include<arpa/inet.h> // for inet_ntoa()
#include<net/ethernet.h>
#include<netinet/ip_icmp.h>   //Provides declarations for icmp header
#include<netinet/udp.h>   //Provides declarations for udp header
#include<netinet/tcp.h>   //Provides declarations for tcp header
#include<netinet/ip.h>    //Provides declarations for ip header
 
void process_packet(u_char *, const struct pcap_pkthdr *, const u_char *);
void process_ip_packet(const u_char * , int);
void print_ip_packet(const u_char * , int);
void print_tcp_packet(const u_char *  , int );
void print_udp_packet(const u_char * , int);
void print_icmp_packet(const u_char * , int );
void PrintData (const u_char * , int);
 
struct sockaddr_in source,dest;
int tcp=0,udp=0,icmp=0,others=0,igmp=0,total=0,i,j; 
 
int main()
{
    pcap_if_t *alldevsp , *device;
  
  //char filter_exp[] = " ";
	char filter_exp[] = "udp";	/* filter expression [3] */
	struct bpf_program fp;			/* compiled filter program (expression) */
	bpf_u_int32 mask;			/* subnet mask */
	bpf_u_int32 net;	
    pcap_t *handle; //Handle of the device that shall be sniffed
 
    char errbuf[100] , *devname , devs[100][100];
    int count = 1 , n;
     
    //First get the list of available devices
    printf("Finding available devices ... ");
    if( pcap_findalldevs( &alldevsp , errbuf) )
    {
        printf("Error finding devices : %s" , errbuf);
        exit(1);
    }
    printf("Done");
     
    //Print the available devices
    printf("\nAvailable Devices are :\n");
    for(device = alldevsp ; device != NULL ; device = device->next)
    {
        printf("%d. %s - %s\n" , count , device->name , device->description);
        if(device->name != NULL)
        {
            strcpy(devs[count] , device->name);
        }
        count++;
    }
     
    //Ask user which device to sniff
    printf("Enter the number of the device you want to sniff : ");
    scanf("%d" , &n);
    devname = devs[n];
     
    //Open the device for sniffing
    printf("Opening device %s for sniffing ... " , devname);
    handle = pcap_open_live(devname , 65536 , 1 , 0 , errbuf);
     
    if (handle == NULL) 
    {
        fprintf(stderr, "Couldn't open device %s : %s\n" , devname , errbuf);
        exit(1);
    }
    printf("Done\n");
  if (pcap_lookupnet(devname, &net, &mask, errbuf) == -1) {
		fprintf(stderr, "Couldn't get netmask for device %s: %s\n",
		    devname, errbuf);
		net = 0;
		mask = 0;
	}

     

     if (pcap_compile(handle, &fp, filter_exp, 0, net) == -1) {
		fprintf(stderr, "Couldn't parse filter %s: %s\n",
		    filter_exp, pcap_geterr(handle));
		exit(EXIT_FAILURE);
	}

	/* apply the compiled filter */
	if (pcap_setfilter(handle, &fp) == -1) {
		fprintf(stderr, "Couldn't install filter %s: %s\n",
		    filter_exp, pcap_geterr(handle));
		exit(EXIT_FAILURE);
	}

    //Put the device in sniff loop
    pcap_loop(handle , 10 , process_packet , NULL);
     
    return 0;   
}
 
void process_packet(u_char *args, const struct pcap_pkthdr *header, const u_char *buffer)
{
    int size = header->len;
     
    //Get the IP Header part of this packet , excluding the ethernet header
    struct iphdr *iph = (struct iphdr*)(buffer + sizeof(struct ethhdr));
    ++total;
    switch (iph->protocol) //Check the Protocol and do accordingly...
    {
        case 1:  //ICMP Protocol
            ++icmp;
            print_icmp_packet( buffer , size);
            break;
         
        case 2:  //IGMP Protocol
            ++igmp;
            break;
         
        case 6:  //TCP Protocol
            ++tcp;
            print_tcp_packet(buffer , size);
            break;
         
        case 17: //UDP Protocol
            ++udp;
            print_udp_packet(buffer , size);
            break;
         
        default: //Some Other Protocol like ARP etc.
            ++others;
            break;
    }
    printf("\nTCP : %d   UDP : %d   ICMP : %d   IGMP : %d   Others : %d   Total : %d\n", tcp , udp , icmp , igmp , others , total);
}
 
void print_ethernet_header(const u_char *Buffer, int Size)
{
    struct ethhdr *eth = (struct ethhdr *)Buffer;
     
    fprintf(stdout , "\n");
    fprintf(stdout , "Ethernet Header\n");
    fprintf(stdout , "   |-Destination Address : %.2X-%.2X-%.2X-%.2X-%.2X-%.2X \n", eth->h_dest[0] , eth->h_dest[1] , eth->h_dest[2] , eth->h_dest[3] , eth->h_dest[4] , eth->h_dest[5] );
    fprintf(stdout , "   |-Source Address      : %.2X-%.2X-%.2X-%.2X-%.2X-%.2X \n", eth->h_source[0] , eth->h_source[1] , eth->h_source[2] , eth->h_source[3] , eth->h_source[4] , eth->h_source[5] );
    fprintf(stdout , "   |-Protocol            : %u \n",(unsigned short)eth->h_proto);
}
 
void print_ip_header(const u_char * Buffer, int Size)
{
    print_ethernet_header(Buffer , Size);
   
    unsigned short iphdrlen;
         
    struct iphdr *iph = (struct iphdr *)(Buffer  + sizeof(struct ethhdr) );
    iphdrlen =iph->ihl*4;
     
    memset(&source, 0, sizeof(source));
    source.sin_addr.s_addr = iph->saddr;
     
    memset(&dest, 0, sizeof(dest));
    dest.sin_addr.s_addr = iph->daddr;
     
    fprintf(stdout , "\n");
    fprintf(stdout , "	IP Header\n");
    fprintf(stdout , "%d	",(unsigned int)iph->version);
    fprintf(stdout , "%d	",((unsigned int)(iph->ihl))*4);
    fprintf(stdout , "%d	",(unsigned int)iph->tos);
    fprintf(stdout , "%d\n",ntohs(iph->tot_len));
    fprintf(stdout , "%d	\n",ntohs(iph->id));
   // fprintf(stdout , "%d ",(unsigned int)iph->ip_reserved_zero);
   // fprintf(stdout , "%d ",(unsigned int)iph->ip_dont_fragment);
   // fprintf(stdout , "%d\n",(unsigned int)iph->ip_more_fragment);
    fprintf(stdout , "%d	",(unsigned int)iph->ttl);
    fprintf(stdout , "%d	",(unsigned int)iph->protocol);
    fprintf(stdout , "  %d\n",ntohs(iph->check));
    fprintf(stdout , "	%s\n" , inet_ntoa(source.sin_addr) );
    fprintf(stdout , "	%s\n" , inet_ntoa(dest.sin_addr) );
}
 
void print_tcp_packet(const u_char * Buffer, int Size)
{
    unsigned short iphdrlen;
     
    struct iphdr *iph = (struct iphdr *)( Buffer  + sizeof(struct ethhdr) );
    iphdrlen = iph->ihl*4;
     
    struct tcphdr *tcph=(struct tcphdr*)(Buffer + iphdrlen + sizeof(struct ethhdr));
             
    int header_size =  sizeof(struct ethhdr) + iphdrlen + tcph->doff*4;
     
    fprintf(stdout , "\n\n***********************TCP Packet*************************\n");  
         
    print_ip_header(Buffer,Size);
         
    fprintf(stdout , "\n");
    fprintf(stdout , "TCP Header\n");
    fprintf(stdout , "   |-Source Port      : %u\n",ntohs(tcph->source));
    fprintf(stdout , "   |-Destination Port : %u\n",ntohs(tcph->dest));
    fprintf(stdout , "   |-Sequence Number    : %u\n",ntohl(tcph->seq));
    fprintf(stdout , "   |-Acknowledge Number : %u\n",ntohl(tcph->ack_seq));
    fprintf(stdout , "   |-Header Length      : %d DWORDS or %d BYTES\n" ,(unsigned int)tcph->doff,(unsigned int)tcph->doff*4);
    //fprintf(stdout , "   |-CWR Flag : %d\n",(unsigned int)tcph->cwr);
    //fprintf(stdout , "   |-ECN Flag : %d\n",(unsigned int)tcph->ece);
    fprintf(stdout , "   |-Urgent Flag          : %d\n",(unsigned int)tcph->urg);
    fprintf(stdout , "   |-Acknowledgement Flag : %d\n",(unsigned int)tcph->ack);
    fprintf(stdout , "   |-Push Flag            : %d\n",(unsigned int)tcph->psh);
    fprintf(stdout , "   |-Reset Flag           : %d\n",(unsigned int)tcph->rst);
    fprintf(stdout , "   |-Synchronise Flag     : %d\n",(unsigned int)tcph->syn);
    fprintf(stdout , "   |-Finish Flag          : %d\n",(unsigned int)tcph->fin);
    fprintf(stdout , "   |-Window         : %d\n",ntohs(tcph->window));
    fprintf(stdout , "   |-Checksum       : %d\n",ntohs(tcph->check));
    fprintf(stdout , "   |-Urgent Pointer : %d\n",tcph->urg_ptr);
    fprintf(stdout , "\n");
    fprintf(stdout , "                        DATA Dump                         ");
    fprintf(stdout , "\n");
         
         
    fprintf(stdout , "\nData Payload\n");    
    PrintData(Buffer + header_size , Size - header_size );
                         
    fprintf(stdout , "\n###########################################################");
}
 
void print_udp_packet(const u_char *Buffer , int Size)
{
     
    unsigned short iphdrlen;
     
    struct iphdr *iph = (struct iphdr *)(Buffer +  sizeof(struct ethhdr));
    iphdrlen = iph->ihl*4;
     
    struct udphdr *udph = (struct udphdr*)(Buffer + iphdrlen  + sizeof(struct ethhdr));
     
    int header_size =  sizeof(struct ethhdr) + iphdrlen + sizeof udph;
     
    fprintf(stdout , "\n\n***********************UDP Packet*************************\n");
     
    print_ip_header(Buffer,Size);           
     fprintf(stdout , "\n");
    fprintf(stdout , "	UDP Header\n");
    fprintf(stdout , "	%d	" , ntohs(udph->source));
    fprintf(stdout , "%d\n" , ntohs(udph->dest));
    fprintf(stdout , "	%d	" , ntohs(udph->len));
    fprintf(stdout , "%d\n" , ntohs(udph->check));
     
  
    fprintf(stdout , "\nData Payload\n");    
     
    //Move the pointer ahead and reduce the size of string
    PrintData(Buffer + header_size , Size - header_size);
     
    fprintf(stdout , "\n###########################################################");
}
 
void print_icmp_packet(const u_char * Buffer , int Size)
{
    unsigned short iphdrlen;
     
    struct iphdr *iph = (struct iphdr *)(Buffer  + sizeof(struct ethhdr));
    iphdrlen = iph->ihl * 4;
     
    struct icmphdr *icmph = (struct icmphdr *)(Buffer + iphdrlen  + sizeof(struct ethhdr));
     
    int header_size =  sizeof(struct ethhdr) + iphdrlen + sizeof icmph;
     
    fprintf(stdout , "\n\n***********************ICMP Packet*************************\n"); 
     
    print_ip_header(Buffer , Size);
             
    fprintf(stdout , "\n");
         
    fprintf(stdout , "ICMP Header\n");
    fprintf(stdout , "   |-Type : %d",(unsigned int)(icmph->type));
             
    if((unsigned int)(icmph->type) == 11)
    {
        fprintf(stdout , "  (TTL Expired)\n");
    }
    else if((unsigned int)(icmph->type) == ICMP_ECHOREPLY)
    {
        fprintf(stdout , "  (ICMP Echo Reply)\n");
    }
     
    fprintf(stdout , "   |-Code : %d\n",(unsigned int)(icmph->code));
    fprintf(stdout , "   |-Checksum : %d\n",ntohs(icmph->checksum));
    //fprintf(stdout , "   |-ID       : %d\n",ntohs(icmph->id));
    //fprintf(stdout , "   |-Sequence : %d\n",ntohs(icmph->sequence));
    fprintf(stdout , "\n");
 
         
    fprintf(stdout , "\nData Payload\n");    
     
    //Move the pointer ahead and reduce the size of string
    PrintData(Buffer + header_size , (Size - header_size) );
     
    fprintf(stdout , "\n###########################################################");
}
 
void PrintData (const u_char * data , int Size)
{
    int i , j;
    for(i=0 ; i < Size ; i++)
    {
        if( i!=0 && i%16==0)   //if one line of hex printing is complete...
        {
            fprintf(stdout , "         ");
            for(j=i-16 ; j<i ; j++)
            {
                if(data[j]>=32 && data[j]<=128)
                    fprintf(stdout , "%c",(unsigned char)data[j]); //if its a number or alphabet
                 
                else fprintf(stdout , "."); //otherwise print a dot
            }
            fprintf(stdout , "\n");
        } 
         
        if(i%16==0) fprintf(stdout , "   ");
            fprintf(stdout , " %02X",(unsigned int)data[i]);
                 
        if( i==Size-1)  //print the last spaces
        {
            for(j=0;j<15-i%16;j++) 
            {
              fprintf(stdout , "   "); //extra spaces
            }
             
            fprintf(stdout , "         ");
             
            for(j=i-i%16 ; j<=i ; j++)
            {
                if(data[j]>=32 && data[j]<=128) 
                {
                  fprintf(stdout , "%c",(unsigned char)data[j]);
                }
                else
                {
                  fprintf(stdout , ".");
                }
            }
             
            fprintf(stdout ,  "\n" );
        }
    }
}
