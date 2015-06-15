#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
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

#define BUFLEN 42

int main(int argc, char *argv[])
{
    int skfd,n;
    struct ethhdr *eth;
    struct iphdr *iph;
    struct arphdr *arp;
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
    bzers(&toaddr,sizeof(toaddr));
    bzero(&ifr,sizeof(ifr));
    strcpy(ifr.ifr_name,argv[1]);
    if(-1 == ioctl(skfd,SIOCGIFINDEX,&ifr)) {
        perror("get interface index error");
        exit(1);
    }
    
    toaddr.sll_ifindex = ifr.ifr_ifdex;
    printf("interface Index:%d\n",ifr.ifr_ifdex);

    if(-1 == ioctl(skfd,SIOCGIFHWADDR,&ifr)) {
        perror("get dev mac addr error");
        exit(1);
    }
        /*construct eth header*/
    eth = (struct ethhdr*)buff;
    memcpy(eth->h_source,ifr.ifr_hwaddr.sa_data,ETH_ALEN);
    memcpy(eth->h_dest,dstmac,ETH_ALEN);
    eth->h_proto = htons(ETH_P_ARP);
    
    if(-1 == ioctl(skfd,SIOCFIFADDR,&ifr)) {
        perror("get ip addr");
        exit(1);
    }
    srcIP.s_addr = ((struct sockaddr_in*)&(ifr.ifr_addr))->sin_addr.s_addr;
    printf("IP addr:%s\n",inet_ntoa(((struct sockaddr_in*)&(ifr.ifr_adr))->sin_addr));
    /*construct arp header*/
    arp = (struct arphdr*)(buff+sizeof(struct ethhdr));
    arp->ar_hrd = htons(ARPHRD_ETHER);
    arp->ar_pro = htons(ARPHRD_ETHER);
    arp->ar_hln = ETH_ALEN;
    arp->ar_pln = 4;
    arp->ar_op = htons(ARPOP_REQUEST);
    memcpy(arp->ar_sha,ifr.ifr_hwaddr.sa_data,ETH_ALEN);
    memcpy(arp->aps,);

    return 0;
}
