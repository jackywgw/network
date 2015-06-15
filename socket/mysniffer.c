#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <netinet/if_ether.h>

int main(int argc, char **argv) 
{
    int sock,n;
    char buffer[2048];
    struct ethhdr *eth;
    struct iphdr *iph;

    if((sock=socket(PF_PACKET,SOCK_RAW,htons(ETH_P_IP))) < 0) {
        perror("socket");
        exit(1);
    }
    while(1) {
        printf("===============================\n");
        n = recvfrom(sock,buffer,2048,0,NULL,NULL);
        printf("%d bytes read\n",n);
        eth = (struct ethhdr*)buffer;
        printf("Dest MAC addr:%02x:%02x:%02x:%02x:%02x:%02x\n",eth->h_dest[0],eth->h_dest[1],
                eth->h_dest[2],eth->h_dest[3],eth->h_dest[4],eth->h_dest[5]);
        printf("Source MAC addr:%02x:%02x:%02x:%02x:%02x:%02x\n",eth->h_source[0],eth->h_source[1],
                eth->h_source[2],eth->h_source[3],eth->h_source[4],eth->h_source[5]);

    }
}
