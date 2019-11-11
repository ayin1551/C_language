#include <getopt.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include "csapp.h"
#include "debug.h"
#include "client_registry.h"
#include "transaction.h"
#include "store.h"
#include "server.h"
#include "protocol.h"
#include "data.h"

static void terminate(int status);

CLIENT_REGISTRY *client_registry;


void handle_signal(int signal){
    if(signal==SIGHUP){
        terminate(EXIT_SUCCESS);
    }

    return;
}

int main(int argc, char* argv[]){
    // Option processing should be performed here.
    // Option '-p <port>' is required in order to specify the port number
    // on which the server should listen.

    int listenfd, *connfdp;
    socklen_t clientlen;
    struct sockaddr_storage clientaddr;
    pthread_t tid;


    char optval;
    int port=-1;

    struct sigaction action={.sa_flags=0,.sa_handler=handle_signal};


    sigaction(SIGHUP,&action,NULL);


    if(argc==1){
        fprintf(stderr, "Port must be specified\n");
        exit(EXIT_FAILURE);
    }
    while(optind < argc) {
        if(((optval = getopt(argc, argv, "p:")) != -1)) {
            switch(optval) {
            case 112:
                port=atoi(optarg);
                break;
            case '?':
                fprintf(stderr, "Port must be specified\n");
                exit(EXIT_FAILURE);
                break;
            default:
                break;
            }

        }
    }

    if(port==-1){
        fprintf(stderr, "Port must be specified\n");
        exit(EXIT_FAILURE);
    }



    client_registry = creg_init();
    trans_init();
    store_init();

    listenfd = Open_listenfd(port);

    while(1){
        clientlen = sizeof(struct sockaddr_storage);
        connfdp = malloc(sizeof(int));
        *connfdp = accept(listenfd,(SA *)&clientaddr,&clientlen);
        Pthread_create(&tid,NULL,xacto_client_service,connfdp);
    }

    // Perform required initializations of the client_registry,
    // transaction manager, and object store.


    // TODO: Set up the server socket and enter a loop to accept connections
    // on this socket.  For each connection, a thread should be started to
    // run function xacto_client_service().  In addition, you should install
    // a SIGHUP handler, so that receipt of SIGHUP will perform a clean
    // shutdown of the server.

    fprintf(stderr, "You have to finish implementing main() "
	    "before the Xacto server will function.\n");

    terminate(EXIT_FAILURE);
}

/*
 * Function called to cleanly shut down the server.
 */
void terminate(int status) {
    // Shutdown all client connections.
    // This will trigger the eventual termination of service threads.

    creg_shutdown_all(client_registry);
    debug("Waiting for service threads to terminate...");

    creg_wait_for_empty(client_registry);

    debug("All service threads terminated.");

    // Finalize modules.
    creg_fini(client_registry);
    trans_fini();
    store_fini();

    debug("Xacto server terminating");
    exit(status);
}
