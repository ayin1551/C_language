#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include "transaction.h"


/*
 * For debugging purposes, all transactions having a nonzero reference count
 * are maintained in a circular, doubly linked list which has the following
 * sentinel as its head.
 */
TRANSACTION trans_list;
int counter;

/*
 * Initialize the transaction manager.
 */
void trans_init(void){
    counter=0;
    trans_list.mutex = (pthread_mutex_t)PTHREAD_MUTEX_INITIALIZER;
    pthread_mutex_lock(&trans_list.mutex);
    trans_list.id=counter;
    sem_init(&trans_list.sem,0,1);
    trans_list.status=TRANS_PENDING;
    trans_list.refcnt=0;
    trans_list.waitcnt=0;
    trans_list.depends=NULL;
    trans_list.next=&trans_list;
    trans_list.prev=&trans_list;
    counter++;
    pthread_mutex_unlock(&trans_list.mutex);
}

/*
 * Finalize the transaction manager.
 */
void trans_fini(void){
    TRANSACTION *a = trans_list.next;
    while(a!=&trans_list){
        TRANSACTION *freetrans = a;
        a = a->next;

        DEPENDENCY *freedepends = freetrans->depends;
        DEPENDENCY *d = freetrans->depends;
        while(d!=NULL){
            freedepends=d;
            d=d->next;
            free(freedepends);

        }
        free(freetrans);

    }
}

/*
 * Create a new transaction.
 *
 * @return  A pointer to the new transaction (with reference count 1)
 * is returned if creation is successful, otherwise NULL is returned.
 */
TRANSACTION *trans_create(void){
    TRANSACTION *a = (TRANSACTION*)malloc(sizeof(TRANSACTION));
    if(a==NULL){
        return NULL;
    }
    a->mutex = (pthread_mutex_t)PTHREAD_MUTEX_INITIALIZER;
    pthread_mutex_lock(&a->mutex);
    sem_init(&a->sem,0,1);
    a->id=counter;
    counter++;
    a->refcnt=1;
    a->waitcnt=0;
    a->status=TRANS_PENDING;
    a->depends=NULL;
    a->next = &trans_list;
    (trans_list.prev)->next = a;
    a->prev = trans_list.prev;
    trans_list.prev = a;

    pthread_mutex_unlock(&a->mutex);
    return a;
}

/*
 * Increase the reference count on a transaction.
 *
 * @param tp  The transaction.
 * @param why  Short phrase explaining the purpose of the increase.
 * @return  The transaction pointer passed as the argument.
 */
TRANSACTION *trans_ref(TRANSACTION *tp, char *why){
    pthread_mutex_lock(&tp->mutex);
    TRANSACTION *a = tp;
    a->refcnt+=1;
    pthread_mutex_unlock(&tp->mutex);
    return a;
}

/*
 * Decrease the reference count on a transaction.
 * If the reference count reaches zero, the transaction is freed.
 *
 * @param tp  The transaction.
 * @param why  Short phrase explaining the purpose of the decrease.
 */
void trans_unref(TRANSACTION *tp, char *why){
    pthread_mutex_lock(&tp->mutex);
    if(tp->refcnt-1==0){
        tp->prev->next = tp->next;
        tp->next->prev = tp->prev;
        if(tp->depends!=NULL){
            DEPENDENCY *freedepends = tp->depends;
            DEPENDENCY *d = tp->depends;
            while(d!=NULL){
                freedepends=d;
                d=d->next;
                free(freedepends);
            }
        }
        pthread_mutex_unlock(&tp->mutex);
        free(tp);

        //free dependency??
    }
    else{
        tp->refcnt-=1;
        pthread_mutex_unlock(&tp->mutex);
    }

}

/*
 * Add a transaction to the dependency set for this transaction.
 *
 * @param tp  The transaction to which the dependency is being added.
 * @param dtp  The transaction that is being added to the dependency set.
 */
void trans_add_dependency(TRANSACTION *tp, TRANSACTION *dtp){
    DEPENDENCY *a = (DEPENDENCY*)malloc(sizeof(DEPENDENCY));
    pthread_mutex_lock(&tp->mutex);
    pthread_mutex_lock(&dtp->mutex);
    if(tp->depends==NULL){
        tp->depends = a;
        a->trans = dtp;
        a->next=NULL;
    }else{
        a->next=tp->depends;
        a->trans=dtp;
        tp->depends = a;
    }
    dtp->waitcnt+=1;
    dtp->refcnt+=1;
    pthread_mutex_unlock(&dtp->mutex);
    pthread_mutex_unlock(&tp->mutex);
}

/*
 * Try to commit a transaction.  Committing a transaction requires waiting
 * for all transactions in its dependency set to either commit or abort.
 * If any transaction in the dependency set abort, then the dependent
 * transaction must also abort.  If all transactions in the dependency set
 * commit, then the dependent transaction may also commit.
 *
 * In all cases, this function consumes a single reference to the transaction
 * object.
 *
 * @param tp  The transaction to be committed.
 * @return  The final status of the transaction: either TRANS_ABORTED,
 * or TRANS_COMMITTED.
 */
TRANS_STATUS trans_commit(TRANSACTION *tp){
    pthread_mutex_lock(&tp->mutex);
    int flag=0;
    DEPENDENCY *d = tp->depends;
    if(d==NULL){
        tp->status = TRANS_COMMITTED;
        pthread_mutex_unlock(&tp->mutex);
        trans_unref(tp,"COMMIT");
        return TRANS_COMMITTED;
    }else{
        while(d!=NULL){
            if(d->trans->status==TRANS_ABORTED){
                tp->status = TRANS_ABORTED;
                pthread_mutex_unlock(&tp->mutex);
                return TRANS_ABORTED;
            }else if(d->trans->status!=TRANS_COMMITTED){
                flag=1;
            }
            if(d->next==NULL){
                if(flag==0){
                    tp->status=TRANS_COMMITTED;
                    pthread_mutex_unlock(&tp->mutex);
                    trans_unref(tp,"COMMIT");
                    return TRANS_COMMITTED;
                }else{
                    d = tp->depends;
                    flag=0;
                }
            }else{
                d = d->next;
            }
        }
    }
    pthread_mutex_unlock(&tp->mutex);
    trans_unref(tp,"COMMIT");
    return TRANS_COMMITTED;
}

/*
 * Abort a transaction.  If the transaction has already committed, it is
 * a fatal error and the program crashes.  If the transaction has already
 * aborted, no change is made to its state.  If the transaction is pending,
 * then it is set to the aborted state, and any transactions dependent on
 * this transaction must also abort.
 *
 * In all cases, this function consumes a single reference to the transaction
 * object.
 *
 * @param tp  The transaction to be aborted.
 * @return  TRANS_ABORTED.
 */
TRANS_STATUS trans_abort(TRANSACTION *tp){
    pthread_mutex_lock(&tp->mutex);
    if(tp->status==TRANS_COMMITTED){
        exit(-1);
    }else if(tp->status==TRANS_ABORTED){
    }else if(tp->status==TRANS_PENDING){
        tp->status=TRANS_ABORTED;
        TRANSACTION *a = &trans_list;
        while(a->next!=&trans_list){
            if(a->next->id==tp->id){
                a = a->next;
                continue;
            }
            if(a->next->depends!=NULL){
                DEPENDENCY *d = a->next->depends;
                if(d->trans->id==tp->id){
                    trans_abort(a->next);
                    a=a->next;
                    continue;
                }
                while(d->next!=NULL){
                    if(d->next->trans->id==tp->id){
                        trans_abort(a->next);
                        break;
                    }
                }
            }
            a = a->next;
        }
    }
    pthread_mutex_unlock(&tp->mutex);
    trans_unref(tp,"ABORT");
    return TRANS_ABORTED;
}

/*
 * Get the current status of a transaction.
 * If the value returned is TRANS_PENDING, then we learn nothing,
 * because unless we are holding the transaction mutex the transaction
 * could be aborted at any time.  However, if the value returned is
 * either TRANS_COMMITTED or TRANS_ABORTED, then that value is the
 * stable final status of the transaction.
 *
 * @param tp  The transaction.
 * @return  The status of the transaction, as it was at the time of call.
 */
TRANS_STATUS trans_get_status(TRANSACTION *tp){
    pthread_mutex_lock(&tp->mutex);
    TRANS_STATUS a = tp->status;
    pthread_mutex_unlock(&tp->mutex);
    return a;
}

/*
 * Print information about a transaction to stderr.
 * No locking is performed, so this is not thread-safe.
 * This should only be used for debugging.
 *
 * @param tp  The transaction to be shown.
 */
void trans_show(TRANSACTION *tp){
    TRANSACTION *a = &trans_list;
    while(a->next!=&trans_list){
        if((a->next)->id==tp->id){
            fprintf(stderr,"[id=%d, status=%d, refcnt=%d]\n",a->next->id,a->next->status,a->next->refcnt);
            break;
        }
    }
}

/*
 * Print information about all transactions to stderr.
 * No locking is performed, so this is not thread-safe.
 * This should only be used for debugging.
 */
void trans_show_all(void){
    TRANSACTION *a = &trans_list;
    while(a->next!=&trans_list){
        if(a->next->id!=0)
            fprintf(stderr,"[id=%d, status=%d, refcnt=%d]",a->next->id,a->next->status,a->next->refcnt);
        a = a->next;
    }
    fprintf(stderr,"\n");
}