#include "data.h"
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>
/*
 * Create a blob with given content and size.
 * The content is copied, rather than shared with the caller.
 * The returned blob has one reference, which becomes the caller's
 * responsibility.
 *
 * @param content  The content of the blob.
 * @param size  The size in bytes of the content.
 * @return  The new blob, which has reference count 1.
 */
BLOB *blob_create(char *content, size_t size){
    BLOB *a =(BLOB*)malloc(sizeof(BLOB));
    a->mutex = (pthread_mutex_t)PTHREAD_MUTEX_INITIALIZER;
    a->content = (char*)malloc(size);
    a->prefix = (char*)malloc(size);
    strncpy(a->content,content,size);
    strncpy(a->prefix,content,size);
    a->size = size;
    a->refcnt = 0;
    blob_ref(a,"test");
    return a;
}

/*
 * Increase the reference count on a blob.
 *
 * @param bp  The blob.
 * @param why  Short phrase explaining the purpose of the increase.
 * @return  The blob pointer passed as the argument.
 */
BLOB *blob_ref(BLOB *bp, char *why){
    pthread_mutex_lock(&bp->mutex);
    BLOB *a = bp;
    bp->refcnt+=1;
    pthread_mutex_unlock(&bp->mutex);
    return a;
}

/*
 * Decrease the reference count on a blob.
 * If the reference count reaches zero, the blob is freed.
 *
 * @param bp  The blob.
 * @param why  Short phrase explaining the purpose of the decrease.
 */
void blob_unref(BLOB *bp, char *why){
    pthread_mutex_lock(&bp->mutex);
    if(bp->refcnt-1==0){
        free(bp->content);
        free(bp);
    }
    else{
        bp->refcnt-=1;
    }
    pthread_mutex_unlock(&bp->mutex);
    return;
}

/*
 * Compare two blobs for equality of their content.
 *
 * @param bp1  The first blob.
 * @param bp2  The second blob.
 * @return 0 if the blobs have equal content, nonzero otherwise.
 */
int blob_compare(BLOB *bp1, BLOB *bp2){
    if(strcmp(bp1->content,bp2->content)==0){
        return 0;
    }
    else{
        return -1;
    }

}

/*
 * Hash function for hashing the content of a blob.
 *
 * @param bp  The blob.
 * @return  Hash of the blob.
 */
int blob_hash(BLOB *bp){
    char *str = bp->content;
    int c;
    int hash=0;
    int length = strlen(str);
    c = *str;
    while(c!=0){
        hash +=(c+length);
        c=*++str;
    }
    return hash;
}

/*
 * Create a key from a blob.
 * The key inherits the caller's reference to the blob.
 *
 * @param bp  The blob.
 * @return  the newly created key.
 */
KEY *key_create(BLOB *bp){
    pthread_mutex_lock(&bp->mutex);
    KEY *a = (KEY*)malloc(sizeof(KEY));
    a->hash = blob_hash(bp);

    a->blob = bp;
    bp->refcnt+=1;
    pthread_mutex_unlock(&bp->mutex);
    return a;
}

/*
 * Dispose of a key, decreasing the reference count of the contained blob.
 * A key must be disposed of only once and must not be referred to again
 * after it has been disposed.
 *
 * @param kp  The key.
 */
void key_dispose(KEY *kp){
    BLOB *bp= kp->blob;
    pthread_mutex_lock(&bp->mutex);
    if(bp->refcnt-1==0){
        free(bp->content);
        free(bp->prefix);
        free(bp);
    }else{
        bp->refcnt-=1;
    }
    free(kp);
    pthread_mutex_unlock(&bp->mutex);
    return;
}

/*
 * Compare two keys for equality.
 *
 * @param kp1  The first key.
 * @param kp2  The second key.
 * @return  0 if the keys are equal, otherwise nonzero.
 */
int key_compare(KEY *kp1, KEY *kp2){
    if(kp1->hash==kp2->hash){
        if(strcmp(kp1->blob->content,kp2->blob->content)==0){
            return 0;
        }
    }
    return -1;
}

/*
 * Create a version of a blob for a specified creator transaction.
 * The version inherits the caller's reference to the blob.
 * The reference count of the creator transaction is increased to
 * account for the reference that is stored in the version.
 *
 * @param tp  The creator transaction.
 * @param bp  The blob.
 * @return  The newly created version.
 */
VERSION *version_create(TRANSACTION *tp, BLOB *bp){
    if(bp!=NULL){
        pthread_mutex_lock(&bp->mutex);
        pthread_mutex_lock(&tp->mutex);
        VERSION *a = (VERSION*)malloc(sizeof(VERSION));
        a->creator = tp;
        a->blob = bp;
        tp->refcnt +=1;
        bp->refcnt +=1;
        a->next = NULL;
        a->prev = NULL;

        pthread_mutex_unlock(&tp->mutex);
        pthread_mutex_unlock(&bp->mutex);
        return a;
    }else{
        pthread_mutex_lock(&tp->mutex);
        VERSION *a = (VERSION*)malloc(sizeof(VERSION));
        a->creator = tp;
        a->blob = NULL;
        tp->refcnt +=1;
        a->next = NULL;
        a->prev = NULL;

        pthread_mutex_unlock(&tp->mutex);
        return a;
    }
}

/*
 * Dispose of a version, decreasing the reference count of the
 * creator transaction and contained blob.  A version must be
 * disposed of only once and must not be referred to again once
 * it has been disposed.
 *
 * @param vp  The version to be disposed.
 */
void version_dispose(VERSION *vp){
    BLOB *bp = vp->blob;
    TRANSACTION *tp = vp->creator;
    pthread_mutex_lock(&bp->mutex);
    pthread_mutex_lock(&tp->mutex);
    if(bp->refcnt-1==0){
        free(bp->content);
        free(bp);
    }else{
        bp->refcnt -=1;
    }
    tp->refcnt -=1;     //!!!!!!!!!!!!!!!!!!tp biancheng0 task666
    free(vp);
    pthread_mutex_unlock(&tp->mutex);
    pthread_mutex_unlock(&bp->mutex);
    return;
}
