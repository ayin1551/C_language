/**
 * All functions you make for the assignment must be implemented in this file.
 * Do not submit your assignment with a main function in this file.
 * If you submit with a main function in this file, you will get a zero.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "debug.h"
#include "sfmm.h"
#include <errno.h>

void *sf_malloc(size_t size) {
    if(size <= 0){
        return NULL;
    }

    if(sf_mem_start() ==sf_mem_end()){
        sf_mem_grow();
        sf_prologue *prologue =  sf_mem_start();
        prologue->header.info.allocated=1;   
        prologue->footer.info.allocated=1;  
    
        sf_epilogue *epilogue = sf_mem_end()-8;
        epilogue->footer.info.allocated=1; 
        sf_header *first_free = sf_mem_start()+40;
        first_free->info.block_size = (PAGE_SZ-48)>>4;
        first_free->info.prev_allocated = 1;
        first_free->info.allocated = 0;
        sf_footer *first_free_f = sf_mem_end()-16;
        first_free_f->info.block_size = (PAGE_SZ-48)>>4;
        first_free_f->info.allocated = 0;
        first_free_f->info.prev_allocated = 1;

    
    
        sf_free_list_node *new_node=sf_add_free_list(PAGE_SZ-48,sf_free_list_head.next);
        
      
        sf_free_list_head.next = new_node;
        sf_free_list_head.prev = new_node;
        new_node->next = &sf_free_list_head;
        new_node->prev = &sf_free_list_head;
        
        sf_header *new_header;
        new_header = &new_node->head;
        
        
        new_header->links.next = first_free;
        new_header->links.prev = first_free;
        first_free->links.prev = new_header;
        first_free->links.next = new_header;
        
       
    }
    
    sf_free_list_node *t = &sf_free_list_head;
    size_t bs = 0;
    while(t->next!=&sf_free_list_head){

        t = t->next;

        if(size+8<=t->size && (t->head).links.next != &(t->head)){
            if(size+8<32){
                bs = 32;
            }else if((size+8)%16!=0){
                bs = size+8+(16-(size+8)%16);
            }else if((size+8)%16==0){
                bs = size+8;
            }
           
            if(t->size - bs>=32){
             
                sf_header *h = (t->head).links.next;
                void *returnPointer = h;
                
                h->info.block_size = bs>>4;
                h->info.requested_size = size;
                h->info.allocated = 1;
                int a =h->info.block_size;
                a = a<<4;
                void *v = h;
                v = v+a;
                
                sf_header *temp_header = v;
                temp_header->info.prev_allocated = 1;
                temp_header->info.allocated = 0;
                temp_header->info.block_size = (t->size - bs)>>4;
          
                v = v+t->size-bs-8;
                
                sf_footer *temp_footer = v;
                temp_footer->info.block_size = (t->size -bs)>>4;
                temp_footer->info.prev_allocated = 1;
                temp_footer->info.allocated = 0;
                bs = t->size-bs;
                
                (t->head).links.next = ((t->head).links.next)->links.next;
                int flag = 0;
                sf_free_list_node *flag_test_head = &sf_free_list_head;
              
                while(flag_test_head->next !=&sf_free_list_head){
                    if(flag_test_head->size == bs){
                        sf_header *flag_header =  (flag_test_head->head).links.next;
                        temp_header->links.next = flag_header->links.next;
                        temp_header->links.prev = flag_header;
                        (flag_header->links.next)->links.prev = temp_header;
                        flag = 1;
                        break;

                    }else if(flag_test_head->size > bs){
                        break;
                    }
              

                        
                    flag_test_head = flag_test_head->next;
                }
                if(flag !=1){
                    sf_free_list_node *flag_test_head = &sf_free_list_head;

                    while(flag_test_head->next !=&sf_free_list_head){
                        

                        if(flag_test_head->size<bs && (flag_test_head->next)->size>bs){
                  
                            sf_free_list_node *new_node=sf_add_free_list(bs,flag_test_head->next);
                            
                            flag_test_head->next = new_node;
                            new_node->prev = flag_test_head;
                           
                            
                            sf_header *new_header = &new_node->head;
        
        
                            new_header->links.next = temp_header;
                            new_header->links.prev = temp_header;
                            temp_header->links.prev = new_header;
                            temp_header->links.next = new_header;
                            break;
                        }
                      
                        flag_test_head = flag_test_head->next;
                    }
                }
                                
                            
                return returnPointer+8;  
                
                
               
            }else{
                bs = t->size;
            
                sf_header *h = (t->head).links.next;
                void *returnPointer = h;
                
                h->info.block_size = bs>>4;
                h->info.requested_size = size;
                h->info.allocated = 1;
                int a =h->info.block_size;
                a = a<<4;
                void *v = h;
                v = v+a;
                
                h = v;
                v = sf_mem_end()-8;
          
                if(h==v){
                    sf_footer *temp_footer = v;
                    temp_footer->info.prev_allocated = 1;   
                }else{
                    sf_header *temp_header = h;
                    temp_header->info.prev_allocated = 1;
                }
                (t->head).links.next = ((t->head).links.next)->links.next;
                
                return returnPointer+8;
              

            }
                
                
  
        }
        
    }

    if(sf_mem_grow()!=NULL){
        void *p = sf_mem_end();
        sf_epilogue *new_epilogue = p-8;
        new_epilogue->footer.info.allocated=1; 
        p = p-PAGE_SZ-8;

        sf_epilogue *old_epilogue = p;

        if(old_epilogue->footer.info.prev_allocated==1){
 
            sf_header *new_free_page = p;
            new_free_page->info.block_size = PAGE_SZ>>4;
            new_free_page->info.prev_allocated = 1;
            new_free_page->info.allocated = 0;
            p = p+PAGE_SZ-8;
            sf_footer *new_free_page_f=p;
            new_free_page_f->info.block_size = PAGE_SZ>>4;
            new_free_page_f->info.prev_allocated = 1;
            new_free_page_f->info.allocated = 0;
            int flag = 0;
            sf_free_list_node *flag_test_head = &sf_free_list_head;
              
            while(flag_test_head->next !=&sf_free_list_head){
                if(flag_test_head->size == PAGE_SZ){
                    sf_header *flag_header =  (flag_test_head->head).links.next;
                    new_free_page->links.next = flag_header->links.next;
                    new_free_page->links.prev = flag_header;
                    (flag_header->links.next)->links.prev = new_free_page;
                    flag = 1;
                    break;

                }else if(flag_test_head->size > PAGE_SZ){
                    break;
                }
                flag_test_head = flag_test_head->next;
            }
            if(flag !=1){

                sf_free_list_node *flag_test_head = &sf_free_list_head;
                flag_test_head = flag_test_head->next;
                while(flag_test_head!=&sf_free_list_head){
        

                    if((flag_test_head->size<PAGE_SZ && (flag_test_head->next)->size>PAGE_SZ) || (flag_test_head->next == &sf_free_list_head)){
                      
                        sf_free_list_node *new_node=sf_add_free_list(PAGE_SZ,flag_test_head->next);
                            
                        flag_test_head->next = new_node;
                        new_node->prev = flag_test_head;
                           
                            
                        sf_header *new_header = &new_node->head;
        
        
                        new_header->links.next = new_free_page;
                        new_header->links.prev = new_free_page;
                        new_free_page->links.prev = new_header;
                        new_free_page->links.next = new_header;
                        break;
                    }
                      
                    flag_test_head = flag_test_head->next;
                }
            }
            
        }else{
            sf_footer *prev_footer = p-8;
            p=p-(prev_footer->info.block_size<<4);
            sf_header *prev_header = p;
            prev_header->info.block_size = ((prev_header->info.block_size<<4)+PAGE_SZ)>>4;
            p = sf_mem_end()-16;
            sf_footer *new_footer = p;
            new_footer->info.block_size = prev_header->info.block_size;
            new_footer->info.prev_allocated = prev_header->info.prev_allocated;
            new_footer->info.allocated = prev_header->info.allocated;
            (prev_header->links.next)->links.prev = (prev_header->links.prev);
            (prev_header->links.prev)->links.next = (prev_header->links.next);
            int flag = 0;
            sf_free_list_node *flag_test_head = &sf_free_list_head;
              
            while(flag_test_head->next !=&sf_free_list_head){
                if(flag_test_head->size == (prev_header->info.block_size<<4)){
                    sf_header *flag_header =  (flag_test_head->head).links.next;
                    prev_header->links.next = flag_header->links.next;
                    prev_header->links.prev = flag_header;
                    (flag_header->links.next)->links.prev = prev_header;
                    flag = 1;
                    break;

                }else if(flag_test_head->size > (prev_header->info.block_size<<4)){
                    break;
                }
                flag_test_head = flag_test_head->next;
            }
            if(flag !=1){

                sf_free_list_node *flag_test_head = &sf_free_list_head;
                flag_test_head = flag_test_head->next;
                while(flag_test_head!=&sf_free_list_head){
        

                    if(((flag_test_head->size<(prev_header->info.block_size<<4)) && ((flag_test_head->next)->size>(prev_header->info.block_size<<4))) || (flag_test_head->next == &sf_free_list_head)){
                      
                        sf_free_list_node *new_node=sf_add_free_list(prev_header->info.block_size<<4,flag_test_head->next);
                            
                        flag_test_head->next = new_node;
                        new_node->prev = flag_test_head;
                           
                            
                        sf_header *new_header = &new_node->head;
        
        
                        new_header->links.next = prev_header;
                        new_header->links.prev = prev_header;
                        prev_header->links.prev = new_header;
                        prev_header->links.next = new_header;
                        break;
                    }
                      
                    flag_test_head = flag_test_head->next;
                }
            }
            
        }
        return sf_malloc(size);
    }else{
        
        return NULL;
    }
    
    
    return NULL;
}

void sf_free(void *pp) {
    if(pp==NULL){
        abort();
    }
    void *start = sf_mem_start()+40;
    void *end = sf_mem_end()-8;
    if(pp<start || pp>end){
        abort();
    }
    
    void *p = pp-8;
    
    sf_header *header = p;

    
    if(header->info.allocated ==0){
        abort();
    }
   

    if((header->info.block_size<<4) %16!=0 || (header->info.block_size<<4) <32){
        abort();
    }
 
    if(header->info.requested_size+8>(header->info.block_size<<4)){
        abort();
    }
    if(header->info.prev_allocated==0){
        sf_footer *p_footer_test = pp-16;
        if(p_footer_test->info.allocated!=0){
            abort();
        }
        void *temp = p_footer_test;
        temp=temp+8;
        temp = temp -(p_footer_test->info.block_size<<4);
        sf_header *p_header_test = temp;
        if(p_header_test->info.allocated!=0){
            abort();
        }
    }
    p = p+(header->info.block_size<<4);
    sf_header *next_header = p;
    sf_header *final_header;
    
    if(next_header->info.allocated ==1){
        if(header->info.prev_allocated ==1){
      
            header->info.allocated=0;
            header->info.requested_size=0;
            p = p-8;
            sf_footer *footer =p;
      
            next_header->info.prev_allocated = 0;
            footer->info.block_size = header->info.block_size;
            footer->info.prev_allocated = header->info.prev_allocated;
            footer->info.allocated = header->info.allocated;
            footer->info.requested_size = header->info.requested_size;
            final_header = header;
        }else{
  
            header->info.allocated=0;
            p = pp-16;
            sf_footer *prev_footer=p;
            p = p+8-(prev_footer->info.block_size<<4);
            sf_header *prev_header = p;
            prev_header->info.block_size = ((prev_header->info.block_size<<4)+(header->info.block_size<<4))>>4;
            p = p+ (prev_header->info.block_size<<4)-8;  
            sf_footer *footer = p;
            footer->info.block_size = prev_header->info.block_size;
            footer->info.prev_allocated = prev_header->info.prev_allocated;
            footer->info.allocated = prev_header->info.allocated;
            footer->info.requested_size = prev_header->info.requested_size;
            (prev_header->links.next)->links.prev = prev_header->links.prev;
            (prev_header->links.prev)->links.next = prev_header->links.next;
            next_header->info.prev_allocated = 0;
            final_header = prev_header;
     
        }
    }else{
        if(header->info.prev_allocated ==1){

            header->info.allocated=0;
            header->info.requested_size=0;
            header->info.block_size = ((header->info.block_size<<4)+(next_header->info.block_size<<4))>>4;
            p = next_header;
            p = p+(next_header->info.block_size<<4)-8;
            sf_footer *next_footer = p;
            next_footer->info.block_size = header->info.block_size;

            next_footer->info.prev_allocated = header->info.prev_allocated;
            next_footer->info.allocated = header->info.allocated;
            next_footer->info.requested_size = header->info.requested_size;
          
            (next_header->links.next)->links.prev = next_header->links.prev;
            (next_header->links.prev)->links.next = next_header->links.next;
            final_header = header;
        }else{
    
            header->info.allocated=0;
            p = pp-16;
            sf_footer *prev_footer=p;
            p = p+8-(prev_footer->info.block_size<<4);
            sf_header *prev_header = p;
            prev_header->info.block_size = ((prev_header->info.block_size<<4)+(header->info.block_size<<4)+(next_header->info.block_size<<4))>>4;
            p = next_header;
            p = p + (next_header->info.block_size<<4)-8;
            sf_footer *next_footer = p;
            next_footer->info.block_size = prev_header->info.block_size;
            next_footer->info.prev_allocated = prev_header->info.prev_allocated;
            next_footer->info.allocated = prev_header->info.allocated;
            next_footer->info.requested_size = prev_header->info.requested_size;
            (next_header->links.next)->links.prev = next_header->links.prev;
            (next_header->links.prev)->links.next = next_header->links.next;
            (prev_header->links.next)->links.prev = prev_header->links.prev;
            (prev_header->links.prev)->links.next = prev_header->links.next;
            final_header = prev_header;
        }
    }
     
        int flag = 0;
        sf_free_list_node *flag_test_head = &sf_free_list_head;
        flag_test_head = flag_test_head->next;
        while(flag_test_head !=&sf_free_list_head){
            
            if(flag_test_head->size == (final_header->info.block_size<<4)){
                sf_header *flag_header = &(flag_test_head->head);
                (flag_header->links.next)->links.prev = final_header;
                final_header->links.prev = flag_header;
                final_header->links.next = flag_header->links.next;
                flag_header->links.next = final_header;
                flag = 1;
                break;

            }else if(flag_test_head->size > (final_header->info.block_size<<4)){
                break;
            }
            flag_test_head = flag_test_head->next;
        }
        if(flag !=1){
          
            sf_free_list_node *flag_test_head = &sf_free_list_head;
            flag_test_head = flag_test_head->next;
            while(flag_test_head!=&sf_free_list_head){
                if(((flag_test_head->size<(final_header->info.block_size<<4)) && ((flag_test_head->next)->size>(final_header->info.block_size<<4))) || (flag_test_head->next == &sf_free_list_head)){
                    sf_free_list_node *new_node=sf_add_free_list(final_header->info.block_size<<4,flag_test_head->next);
                            
                    flag_test_head->next = new_node;
                    new_node->prev = flag_test_head;
                           
                            
                    sf_header *new_header = &new_node->head;
        
        
                    new_header->links.next = final_header;
                    new_header->links.prev = final_header;
                    final_header->links.prev = new_header;
                    final_header->links.next = new_header;
                    break;
                }else if((flag_test_head->prev == &sf_free_list_head)&&(((flag_test_head)->size)>(final_header->info.block_size<<4))){       

                    sf_free_list_node *new_node=sf_add_free_list(final_header->info.block_size<<4,flag_test_head);
                            
                    sf_free_list_head.next = new_node;
                    new_node->prev = &sf_free_list_head;
                           
                            
                    sf_header *new_header = &new_node->head;
        
        
                    new_header->links.next = final_header;
                    new_header->links.prev = final_header;
                    final_header->links.prev = new_header;
                    final_header->links.next = new_header;
                    break;
                

                }
                
                      
                flag_test_head = flag_test_head->next;
            }
        }
    
    
    return;
}

void *sf_realloc(void *pp, size_t rsize) {
    if(pp==NULL){
        abort();
    }
    void *start = sf_mem_start()+40;
    void *end = sf_mem_end()-8;
    if(pp<start || pp>end){
        abort();
    }
    
    void *p = pp-8;
    
    sf_header *header = p;

    
    if(header->info.allocated ==0){
        abort();
    }
   

    if((header->info.block_size<<4) %16!=0 || (header->info.block_size<<4) <32){
        abort();
    }
 
    if(header->info.requested_size+8>(header->info.block_size<<4)){
        abort();
    }
    if(header->info.prev_allocated==0){
        sf_footer *p_footer_test = pp-16;
        if(p_footer_test->info.allocated!=0){
            abort();
        }
        void *temp = p_footer_test;
        temp=temp+8;
        temp = temp -(p_footer_test->info.block_size<<4);
        sf_header *p_header_test = temp;
        if(p_header_test->info.allocated!=0){
            abort();
        }
    }
    if(rsize==0){
        sf_free(pp);
        return NULL;
    }
    header = pp-8;
    if(header->info.requested_size<rsize){
        p = sf_malloc(rsize);
        if(p==NULL){
            return NULL;
        }
        memcpy(p,pp,header->info.requested_size);
        sf_free(pp);
        return p;
    }else if(header->info.requested_size>rsize){
        int split = rsize+8;
        if(split<32){
            split = 32;
        }else if(split%16!=0){
            split = split + (16-split%16);
        }
        if((header->info.block_size<<4)-split<32){
            header->info.requested_size = rsize;
        }else if((header->info.block_size<<4)-split>=32){
            header->info.requested_size = rsize ;  
            
            p = pp-8;
            p = p+(header->info.block_size<<4);
            sf_header *next_header = p;
            sf_header *final_header;
            if(next_header->info.allocated==1){
                next_header->info.prev_allocated = 0;
                p=pp-8+split;
                sf_header *new_header = p;
                new_header->info.block_size = ((header->info.block_size<<4)-split)>>4;
                new_header->info.requested_size=0;
                new_header->info.allocated = 0;
                new_header->info.prev_allocated = 1;
                p = pp-8;
                p=p+(header->info.block_size<<4);
                p = p-8;
                sf_footer *new_footer = p;
                new_footer->info.block_size = new_header->info.block_size;
                new_footer->info.requested_size=0;
                new_footer->info.allocated = 0;
                new_footer->info.prev_allocated = 1;
                final_header = new_header;
            }else if(next_header->info.allocated==0){
                p=pp-8+split;
                sf_header *new_header = p;
                
                new_header->info.requested_size=0;
                new_header->info.allocated = 0;
                new_header->info.prev_allocated = 1;
                new_header->info.block_size = (((header->info.block_size<<4)-split)+(next_header->info.block_size<<4))>>4;
                p = p+(new_header->info.block_size<<4);
                p = p-8;
                sf_footer *new_footer = p;
                new_footer->info.block_size = new_header->info.block_size;
                new_footer->info.requested_size=0;
                new_footer->info.prev_allocated = 1;
                new_footer->info.allocated = 0;
                (next_header->links.next)->links.prev = next_header->links.prev;
                (next_header->links.prev)->links.next = next_header->links.next;
                final_header = new_header;
            }
            header->info.block_size = split>>4;
            int flag = 0;
            sf_free_list_node *flag_test_head = &sf_free_list_head;
            flag_test_head = flag_test_head->next;
            while(flag_test_head !=&sf_free_list_head){
                
                if(flag_test_head->size == (final_header->info.block_size<<4)){
                    sf_header *flag_header = &(flag_test_head->head);
                    (flag_header->links.next)->links.prev = final_header;
                    final_header->links.prev = flag_header;
                    final_header->links.next = flag_header->links.next;
                    flag_header->links.next = final_header;
                    flag = 1;
                    break;
                }else if(flag_test_head->size > (final_header->info.block_size<<4)){
                    break;
                }
                flag_test_head = flag_test_head->next;
            }
            if(flag !=1){
              
                sf_free_list_node *flag_test_head = &sf_free_list_head;
                flag_test_head = flag_test_head->next;
                while(flag_test_head!=&sf_free_list_head){
                    if(((flag_test_head->size<(final_header->info.block_size<<4)) && ((flag_test_head->next)->size>(final_header->info.block_size<<4))) || (flag_test_head->next == &sf_free_list_head)){
                        sf_free_list_node *new_node=sf_add_free_list(final_header->info.block_size<<4,flag_test_head->next);
                                
                        flag_test_head->next = new_node;
                        new_node->prev = flag_test_head;
                               
                                
                        sf_header *new_header = &new_node->head;
            
            
                        new_header->links.next = final_header;
                        new_header->links.prev = final_header;
                        final_header->links.prev = new_header;
                        final_header->links.next = new_header;
                        break;
                    }else if((flag_test_head->prev == &sf_free_list_head)&&(((flag_test_head)->size)>(final_header->info.block_size<<4))){       

                        sf_free_list_node *new_node=sf_add_free_list(final_header->info.block_size<<4,flag_test_head);
                                
                        sf_free_list_head.next = new_node;
                        new_node->prev = &sf_free_list_head;
                               
                                
                        sf_header *new_header = &new_node->head;
            
            
                        new_header->links.next = final_header;
                        new_header->links.prev = final_header;
                        final_header->links.prev = new_header;
                        final_header->links.next = new_header;
                        break;
                    

                    }
                    
                          
                    flag_test_head = flag_test_head->next;
                }
            }
            
            
        }
        return pp;
            
    }
    
    return pp;
}
