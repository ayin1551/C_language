#include "client_registry.h"
#include <stdio.h>
#include <stdlib.h>
#include <semaphore.h>
#include <sys/socket.h>

typedef struct client_registry{
    int *fdbuf;
    sem_t mutex;
    sem_t slots;
    sem_t items;
    sem_t lock;
}CLIENT_REGISTRY;


CLIENT_REGISTRY *creg_init(){
    CLIENT_REGISTRY *a = (CLIENT_REGISTRY*)malloc(sizeof(CLIENT_REGISTRY));
    a->fdbuf = calloc(1024, sizeof(int));
    for(int i=0;i<1024;i++){
        a->fdbuf[i]=-1;
    }
    sem_init(&a->mutex,0,1);
    sem_init(&a->slots,0,1024);
    sem_init(&a->items,0,0);
    sem_init(&a->lock,0,1);
    return a;
}


void creg_fini(CLIENT_REGISTRY *cr){
    free(cr->fdbuf);
    free(cr);
}

void creg_register(CLIENT_REGISTRY *cr, int fd){
    sem_wait(&cr->lock);
    sem_wait(&cr->slots);
    sem_wait(&cr->mutex);
    for(int i=0;i<1024;i++){
        if(cr->fdbuf[i]==-1){
            cr->fdbuf[i]=fd;
            break;
        }
    }

    sem_post(&cr->mutex);
    sem_post(&cr->items);
    sem_post(&cr->lock);
}

void creg_unregister(CLIENT_REGISTRY *cr, int fd){
    sem_wait(&cr->items);
    sem_wait(&cr->mutex);
    int flag = 0;
    for(int i=0;i<1024;i++){
        if(cr->fdbuf[i]==fd){
            cr->fdbuf[i]=-1;
            flag=1;
            break;
        }
    }
    if(flag==0){
        sem_post(&cr->items);
        sem_post(&cr->mutex);
        return;
    }
    sem_post(&cr->mutex);
    sem_post(&cr->slots);

}


void creg_wait_for_empty(CLIENT_REGISTRY *cr){
    sem_wait(&cr->lock);
    int flag=0;
    int i=0;
    while(i<=1023){
        if(i==0){
            flag=0;
        }
        if(cr->fdbuf[i]!=-1){
            flag=1;
        }

        if(i==1023){
            if(flag==0){
                break;
            }
            else{
                i=0;
            }
        }else{
            i++;
        }
    }

}

void creg_shutdown_all(CLIENT_REGISTRY *cr){
    sem_wait(&cr->mutex);
    for(int i=0;i<1024;i++){
        if(cr->fdbuf[i]!=-1){
            shutdown(cr->fdbuf[i],SHUT_RDWR);
        }
    }
    sem_post(&cr->mutex);
}