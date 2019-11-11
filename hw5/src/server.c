#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include "client_registry.h"
#include "transaction.h"
#include "protocol.h"
#include "csapp.h"
#include "data.h"
#include "store.h"
#include <time.h>

CLIENT_REGISTRY *client_registry;



void *xacto_client_service(void *arg){
    int connfd = *((int*)arg);
    pthread_detach(pthread_self());
    creg_register(client_registry, connfd);
    TRANSACTION *trans =trans_create();
    XACTO_PACKET *pkt = (XACTO_PACKET*)malloc(sizeof(XACTO_PACKET));
    void* datap[2]={};
    int isCommit = 0;
    while(proto_recv_packet(connfd, pkt, datap)==0){
        if(pkt->type==XACTO_GET_PKT){
            proto_recv_packet(connfd, pkt, datap);
            BLOB* keyblob = blob_create(datap[0],strlen(datap[0]));
            KEY* key = key_create(keyblob);
            BLOB* value[2]={};
            trans->status=store_get(trans,key,value);
            struct timespec times;
            clock_gettime(CLOCK_MONOTONIC,&times);
            pkt->timestamp_nsec = times.tv_nsec;
            pkt->timestamp_sec = times.tv_sec;
            pkt->status = trans->status;
            pkt->size=0;
            pkt->null=0;
            pkt->type =XACTO_REPLY_PKT;
            proto_send_packet(connfd,pkt,NULL);
            clock_gettime(CLOCK_MONOTONIC,&times);
            pkt->timestamp_nsec = times.tv_nsec;
            pkt->timestamp_sec = times.tv_sec;
            pkt->status = trans->status;
            pkt->type = XACTO_DATA_PKT;
            pkt->null=0;

            if(value[0]==NULL){
                pkt->null=1;
                pkt->size=0;
                proto_send_packet(connfd,pkt,NULL);
            }else{
                pkt->size = strlen(value[0]->content);
                pkt->null=0;
                proto_send_packet(connfd,pkt,value[0]->content);
            }

            store_show();
            trans_show_all();
        }else if(pkt->type==XACTO_PUT_PKT){
            proto_recv_packet(connfd, pkt, datap);
            BLOB* keyblob = blob_create(datap[0],strlen(datap[0]));
            KEY* key = key_create(keyblob);
            proto_recv_packet(connfd, pkt, datap);
            BLOB* value = blob_create(datap[0],strlen(datap[0]));
            trans->status = store_put(trans,key,value);
            struct timespec times;
            clock_gettime(CLOCK_MONOTONIC,&times);
            pkt->timestamp_nsec = times.tv_nsec;
            pkt->timestamp_sec = times.tv_sec;
            pkt->status = trans->status;
            pkt->type =XACTO_REPLY_PKT;
            pkt->size = 0;
            pkt->null=0;

            proto_send_packet(connfd,pkt,NULL);
            store_show();
            trans_show_all();
        }else if(pkt->type==XACTO_COMMIT_PKT){
            trans->status = trans_commit(trans);
            struct timespec times;
            clock_gettime(CLOCK_MONOTONIC,&times);
            pkt->timestamp_nsec = times.tv_nsec;
            pkt->timestamp_sec = times.tv_sec;
            pkt->status = trans->status;
            pkt->type =XACTO_REPLY_PKT;
            pkt->size = 0;
            pkt->null=0;
            isCommit=1;
            proto_send_packet(connfd,pkt,NULL);
            store_show();
            trans_show_all();

        }

    }

    if(isCommit==0){
        trans_abort(trans);
    }

    creg_unregister(client_registry, connfd);
    free(pkt);
    free(arg);
    close(connfd);
    return NULL;
}
