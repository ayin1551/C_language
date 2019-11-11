#include "protocol.h"
#include "csapp.h"
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h>

int proto_send_packet(int fd, XACTO_PACKET *pkt, void *data){
    uint32_t payload_length = pkt->size;
    pkt->size = htonl(pkt->size);
    pkt->timestamp_sec = htonl(pkt->timestamp_sec);
    pkt->timestamp_nsec = htonl(pkt->timestamp_nsec);

    if(rio_writen(fd,pkt,sizeof(XACTO_PACKET))==-1){
        return -1;
    }
    if(rio_writen(fd,data,payload_length)==-1){
        return -1;
    }

    return 0;
}

int proto_recv_packet(int fd, XACTO_PACKET *pkt, void **datap){
    if(rio_readn(fd,pkt,sizeof(XACTO_PACKET))==-1){
        return -1;
    }

    pkt->size = ntohl(pkt->size);
    pkt->timestamp_sec = ntohl(pkt->timestamp_sec);
    pkt->timestamp_nsec = ntohl(pkt->timestamp_nsec);
    pkt->null=0;

    if(pkt->size==0){
        datap=NULL;
        return 0;
    }


    datap[0]=(char*)malloc(pkt->size);
    if(rio_readn(fd,datap[0],pkt->size)==-1){
        return -1;
    }


    return 0;

}