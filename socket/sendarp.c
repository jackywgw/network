#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <stdio.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <netinet/if_ether.h>
#include <net/if_arp.h>
#include <netpacket/packet.h>
#include <net/if.h>
#include <net/ethernet.h>
#include <arpa/inet.h>
#define BUFLEN 42

/*
************************eth structure**************************************
struct ethhdr {
    unsigned char h_dest[ETH_ALEN];
    unsigned char h_source[ETH_ALEN];
    __be16 h_proto;
} __attribute__((packed));

struct ether_header
{
    u_int8_t ether_dhost[ETH_ALEN];      // destination eth addr 
    u_int8_t ether_shost[ETH_ALEN];      // source ether addr    
    u_int16_t ether_type;                 // packet type ID field 
} __attribute__ ((__packed__));
**************************arp structrure (if_arp.h)**************************
struct arphdr
{
    __be16      ar_hrd;     //format of hardware address
    __be16      ar_pro;     // format of protocol address   *\/
    unsigned char   ar_hln;     // length of hardware address   *\/
    unsigned char   ar_pln;     // length of protocol address   *\/
    __be16      ar_op;      // ARP opcode (command)     *\/

#if 0
    //  Ethernet looks like this : This bit is variable sized however...*\/
    unsigned char       ar_sha[ETH_ALEN];   // sender hardware address  *\/
    unsigned char       ar_sip[4];      // sender IP address        *\/
    unsigned char       ar_tha[ETH_ALEN];   // target hardware address  *\/
    unsigned char       ar_tip[4];      // target IP address        *\/
#endif

};  
*************************ether_arp structure********************************
__BEGIN_DECLS
//
 * Ethernet Address Resolution Protocol.
 *
 * See RFC 826 for protocol description.  Structure below is adapted
 * to resolving internet addresses.  Field names used correspond to
 * RFC 826.
 *\/
 struct    ether_arp {
    struct  arphdr ea_hdr;      // fixed-size header *\/
    u_int8_t arp_sha[ETH_ALEN]; // sender hardware address *\/
    u_int8_t arp_spa[4];        // sender protocol address *\/
    u_int8_t arp_tha[ETH_ALEN]; // target hardware address *\/
    u_int8_t arp_tpa[4];        // target protocol address *\/
 };
#define arp_hrd ea_hdr.ar_hrd
#define arp_pro ea_hdr.ar_pro
#define arp_hln ea_hdr.ar_hln
#define arp_pln ea_hdr.ar_pln
#define arp_op  ea_hdr.ar_op

**************************ifreq structure************************************
struct ifreq
{
#define IFHWADDRLEN 6
    union
    {
        char ifrn_name[IFNAMSIZ];  
    } ifr_ifrn;

    union {
        struct sockaddr ifru_addr;
        struct sockaddr ifru_dstaddr;
        struct sockaddr ifru_broadaddr;
        struct sockaddr ifru_netmask;
        struct  sockaddr ifru_hwaddr;
        short ifru_flags;
        int ifru_ivalue;
        int ifru_mtu;
        struct  ifmap ifru_map;
        char ifru_slave[IFNAMSIZ]; 
        char ifru_newname[IFNAMSIZ];
        void __user * ifru_data;
        struct if_settings ifru_settings;
    } ifr_ifru;
};
#define ifr_name ifr_ifrn.ifrn_name 
#define ifr_hwaddr ifr_ifru.ifru_hwaddr 
#define ifr_addr ifr_ifru.ifru_addr 
#define ifr_dstaddr ifr_ifru.ifru_dstaddr 
#define ifr_broadaddr ifr_ifru.ifru_broadaddr 
#define ifr_netmask ifr_ifru.ifru_netmask 
#define ifr_flags ifr_ifru.ifru_flags 
#define ifr_metric ifr_ifru.ifru_ivalue 
#define ifr_mtu  ifr_ifru.ifru_mtu 
#define ifr_map  ifr_ifru.ifru_map 
#define ifr_slave ifr_ifru.ifru_slave 
#define ifr_data ifr_ifru.ifru_data 
#define ifr_ifindex ifr_ifru.ifru_ivalue 
#define ifr_bandwidth ifr_ifru.ifru_ivalue    
#define ifr_qlen ifr_ifru.ifru_ivalue 
#define ifr_newname ifr_ifru.ifru_newname 
#define ifr_settings ifr_ifru.ifru_settings
 */
int main(int argc, char *argv[])
{
    int skfd,n;
    struct ethhdr *eth;
    struct ether_arp *arp;
    struct ifreq ifr;
    unsigned char buff[BUFLEN];
    unsigned char dst_mac[ETH_ALEN] = {0xff,0xff,0xff,0xff,0xff,0xff};
    struct sockaddr_ll toaddr;
    struct in_addr targetIP,srcIP;

    if(argc != 3) {
        printf("Usage: %s outinterface dstIP\n",argv[0]);
        exit(1);
    }
    skfd = socket(PF_PACKET,SOCK_RAW,htons(ETH_P_ALL));
    if(skfd < 0 ) {
        perror("Create Error");
        exit(1);
    }
    bzero(&toaddr,sizeof(toaddr));
    bzero(&ifr,sizeof(ifr));
    strcpy(ifr.ifr_name,argv[1]);
    if(-1 == ioctl(skfd,SIOCGIFINDEX,&ifr)) {
        perror("get interface index error");
        exit(1);
    }
    
    toaddr.sll_ifindex = ifr.ifr_ifindex;
    printf("interface Index:%d\n",ifr.ifr_ifindex);

    if(-1 == ioctl(skfd,SIOCGIFHWADDR,&ifr)) {
        perror("get dev mac addr error");
        exit(1);
    }
    /*construct eth header*/
    eth = (struct ethhdr*)buff;
    memcpy(eth->h_source,ifr.ifr_hwaddr.sa_data,ETH_ALEN);
    memcpy(eth->h_dest,dst_mac,ETH_ALEN);
    eth->h_proto = htons(ETH_P_ARP);
    
    printf("Src mac is %2x:%2x:%2x:%2x:%2x:%2x\n",eth->h_source[0],eth->h_source[1],eth->h_source[2],
            eth->h_source[3],eth->h_source[4],eth->h_source[5]);
    if(-1 == ioctl(skfd,SIOCGIFADDR,&ifr)) {
        perror("get ip addr");
        exit(1);
    }
    srcIP.s_addr = ((struct sockaddr_in*)&(ifr.ifr_addr))->sin_addr.s_addr;
    printf("IP addr:%s\n",inet_ntoa(((struct sockaddr_in*)&(ifr.ifr_addr))->sin_addr));
  
    /*construct arp header*/
    arp = (struct ether_arp*)(buff+sizeof(struct ethhdr));
    arp->arp_hrd = htons(ARPHRD_ETHER);
    arp->arp_pro = htons(ETHERTYPE_IP);
    arp->arp_hln = ETH_ALEN;
    arp->arp_pln = 4;
    arp->arp_op = htons(ARPOP_REQUEST);
    memcpy(arp->arp_sha,ifr.ifr_hwaddr.sa_data,ETH_ALEN);
    memcpy(arp->arp_spa,&srcIP,sizeof(srcIP));
    memset(arp->arp_tha,0,ETH_ALEN);
    inet_pton(AF_INET,argv[2],&targetIP);
    memcpy(arp->arp_tpa,&targetIP,4);
    toaddr.sll_family = PF_PACKET;

    n = sendto(skfd,buff,BUFLEN,0,(struct sockaddr*)&toaddr,sizeof(toaddr));
    
    printf("send arp request %d bytes\n",n);
    close(skfd);

    return 0;
}
