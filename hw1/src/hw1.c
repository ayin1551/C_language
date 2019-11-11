#include <stdlib.h>

#include "debug.h"
#include "hw1.h"



#ifdef _STRING_H
#error "Do not #include <string.h>. You will get a ZERO."
#endif

#ifdef _STRINGS_H
#error "Do not #include <strings.h>. You will get a ZERO."
#endif

#ifdef _CTYPE_H
#error "Do not #include <ctype.h>. You will get a ZERO."
#endif

/*
 * You may modify this file and/or move the functions contained here
 * to other source files (except for main.c) as you wish.
 *
 * IMPORTANT: You MAY NOT use any array brackets (i.e. [ and ]) and
 * you MAY NOT declare any arrays or allocate any storage with malloc().
 * The purpose of this restriction is to force you to use pointers.
 * Variables to hold the content of three frames of audio data and
 * two annotation fields have been pre-declared for you in const.h.
 * You must use those variables, rather than declaring your own.
 * IF YOU VIOLATE THIS RESTRICTION, YOU WILL GET A ZERO!
 *
 * IMPORTANT: You MAY NOT use floating point arithmetic or declare
 * any "float" or "double" variables.  IF YOU VIOLATE THIS RESTRICTION,
 * YOU WILL GET A ZERO!
 */
unsigned int initnum=0;
AUDIO_HEADER hp;
int *previous_frame_int = (int*)&previous_frame;
int *input_frame_int = (int*)&input_frame;
int *output_frame_int = (int*)&output_frame;
void framecpy(int *dest,int *src,int channels,int byte_per_sample);
void calcu_frame(int *pp,int *ip,int *op,int channels,int bytes_per_sample,int fac,int k);
int annotationcpy(char *dest, char *src);
int strToInt(char *str);
unsigned int strToHex(char *str);

/**
 * @brief Validates command line arguments passed to the program.
 * @details This function will validate all the arguments passed to the
 * program, returning 1 if validation succeeds and 0 if validation fails.
 * Upon successful return, the selected program options will be set in the
 * global variables "global_options", where they will be accessible
 * elsewhere in the program.
 *
 * @param argc The number of arguments passed to the program from the CLI.
 * @param argv The argument strings passed to the program from the CLI.
 * @return 1 if validation succeeds and 0 if validation fails.
 * Refer to the homework document for the effects of this function on
 * global variables.
 * @modifies global variable "global_options" to contain a bitmap representing
 * the selected options.
 */
int validargs(int argc, char **argv)
{
    if(argc==1){
        return 0;
    }else if(argc>1){
        char t;
        char fac;
        char *test;
        long ret;
        t=*(*(argv+1)+1);
        if(t=='h'){
            global_options |=1UL<< 63;            
            return 1;
        }else if(t=='u'){
            global_options |=1UL<< 62;
            if(argc>2){
                t=*(*(argv+2)+1);
                if(t=='f'){
                    if(argc>3){
                        fac=*(*(argv+3)+0);
                        if(fac=='-'){
                            return 0;
                        }else{
                            ret=strToInt(*(argv+3));
                            if(ret>=1 && ret<=1024){
                                ret -=1;
                                global_options |=ret << 48;
                                if(argc==5){
                                    t=*(*(argv+4)+1);
                                    if(t=='p'){
                                        global_options |=1UL<<59;
                                        return 1;
                                    }
                                }else if(argc>5){
                                    return 0;
                                }else{
                                    return 1;
                                }
                                
                            }else{
                                return 0;
                            }
                        }
                    }
                }else if(t=='p'){
                    global_options |=1UL<<59;
                    if(argc>3 && argc==5){
                        t=*(*(argv+3)+1);
                        if(t=='f'){
                            ret=strToInt(*(argv+4));
                            if(ret>=1 && ret<=1024){
                                ret -=1;
                                global_options |=ret << 48;
                                return 1;
                            }else{
                                return 0;
                            }
                        }
                    }else if(argc==3){
                        return 1;
                    }else{
                        return 0;
                    }
        
                }
            }else{
                return 1;
            }
            
        }else if(t=='d'){
            global_options |=1UL<< 61;
            if(argc>2){
                t=*(*(argv+2)+1);
                if(t=='f'){
                    if(argc>3){
                        fac=*(*(argv+3)+0);
                        if(fac=='-'){
                            return 0;
                        }else{
                            ret=strToInt(*(argv+3));
                            if(ret>=1 && ret<=1024){
                                ret -=1;
                                global_options |=ret << 48;
                                if(argc==5){
                                    t=*(*(argv+4)+1);
                                    if(t=='p'){
                                        global_options |=1UL<<59;
                                        return 1;
                                    }
                                }else if(argc>5){
                                    return 0;
                                }else{
                                    return 1;
                                }
                            }else{
                                return 0;
                            }
                        }
                    }
                }else if(t=='p'){
                    global_options |=1UL<<59;
                    if(argc>3 && argc==5){
                        t=*(*(argv+3)+1);
                        if(t=='f'){
                            ret=strToInt(*(argv+4));
                            if(ret>=1 && ret<=1024){
                                ret -=1;
                                global_options |=ret << 48;
                                return 1;
                            }else{
                                return 0;
                            }
                        }
                    }else if(argc==3){
                        return 1;
                    }else{
                        return 0;
                    }
        
                }
            }else{
                return 1;
            }
        }else if(t=='c'){
            int counter=0;
            global_options |=1UL<< 60;
            if(argc>2){
                t=*(*(argv+2)+1);
                if(t=='k' && argc>=4){
                    for(counter=0;counter>=0;counter++){
                        if(*(*(argv+3)+counter)==0){
                            break;
                        }else if(*(*(argv+3)+counter)>=48 && *(*(argv+3)+counter)<=57){
                        }else if(*(*(argv+3)+counter)>=65 && *(*(argv+3)+counter)<=70){
                        }else if(*(*(argv+3)+counter)>=97 && *(*(argv+3)+counter)<=102){
                        }else{
                            return 0;
                        }
                    }
                    if(counter<=8){
                       global_options |=strToHex(*(argv+3));
                    }else{
                       return 0;
                    }
                    if(argc==5 && *(*(argv+4)+1)=='p'){
                        global_options |=1UL<<59;
                    }else if(argc>4){
                        return 0;
                    }
                    return 1;
                        

                }else if(t=='p'){
                    global_options |=1UL<<59;
                    if(argc==5){
                        t=*(*(argv+3)+1);
                        if(t=='k'){
                            for(counter=0;counter>=0;counter++){
                                if(*(*(argv+3)+counter)==0){
                                    break;
                                }else if(*(*(argv+3)+counter)>=48 && *(*(argv+3)+counter)<=57){
                                }else if(*(*(argv+3)+counter)>=65 && *(*(argv+3)+counter)<=70){
                                }else if(*(*(argv+3)+counter)>=97 && *(*(argv+3)+counter)<=102){
                                }else{
                                    return 0;
                                }
                            }
                        }else{
                            return 0;
                        }
                        if(counter<=8){
                            global_options |=strToHex(*(argv+4));
                            return 1;
                        }else{
                            return 0;
                        }
                        
                    }else if(argc>3){
                        return 0;
                    }
 
                }else{
                    return 0;
                }
            }else{
                return 1;
            }
        }else{
            return 0;
        }
        
        return 0;
    }else{
        return 0;
    }
    
    return 0;
}

/**
 * @brief  Recodes a Sun audio (.au) format audio stream, reading the stream
 * from standard input and writing the recoded stream to standard output.
 * @details  This function reads a sequence of bytes from the standard
 * input and interprets it as digital audio according to the Sun audio
 * (.au) format.  A selected transformation (determined by the global variable
 * "global_options") is applied to the audio stream and the transformed stream
 * is written to the standard output, again according to Sun audio format.
 *
 * @param  argv  Command-line arguments, for constructing modified annotation.
 * @return 1 if the recoding completed successfully, 0 otherwise.
 */
int recode(char **argv) {
    char c;
    int flag;

    flag = read_header(&hp);
    if(flag==0){
        return 0;
    }
    unsigned int size = hp.data_offset - sizeof(AUDIO_HEADER);
    if(size>1024){
        return 0;
    }
    flag = read_annotation(input_annotation,size);
    if(flag==0){
        return 0;
    }
    
    if(global_options & (1UL<<59)){
        annotationcpy(output_annotation,input_annotation);
    }else{
        int at=0;
        while(1){
            if((*((*argv)+at))=='\0'){
                break;
            }
            *(output_annotation+at) = *((*argv)+at);
            at++;
        }
        
        if(global_options & (1UL<<62)){
            *(output_annotation+at)= ' ';
            at++;   
            *(output_annotation+at)= '-';
            at++;
            *(output_annotation+at)= 'u';
            at++;
            
            if((*(*(argv+2)+1))=='f'){
                *(output_annotation+at)= ' ';
                at++;
                *(output_annotation+at)= '-';
                at++;
                *(output_annotation+at)= 'f';
                at++;
                *(output_annotation+at)= ' ';
                at++;

                int tempat=0;
                while(1){
                    if((*(*(argv+3)+tempat))=='\0'){
                        break;
                    }

                    *(output_annotation+at) = *(*(argv+3)+tempat);
                    at++;
                    tempat++;
                }
                
            }
                
        }else if(global_options & (1UL<<61)){
            *(output_annotation+at)= ' ';
            at++;
            *(output_annotation+at)= '-';
            at++;
            *(output_annotation+at)= 'd';
            at++;
            if((*(*(argv+2)+1))=='f'){
                *(output_annotation+at)= ' ';
                at++;
                *(output_annotation+at)= '-';
                at++;
                *(output_annotation+at)= 'f';
                at++;
                *(output_annotation+at)= ' ';
                at++;
                int tempat=0;
                while(1){
                    if((*(*(argv+3)+tempat))=='\0'){
                        break;
                    }
                    *(output_annotation+at) = *(*(argv+3)+tempat);
                    at++;
                    tempat++;
                }
            }
        }else if(global_options & (1UL<<60)){
            *(output_annotation+at)= ' ';
            at++;
            *(output_annotation+at)= '-';
            at++;
            *(output_annotation+at)= 'c';
            at++;
            if((*(*(argv+2)+1))=='k'){
                *(output_annotation+at)= ' ';
                at++;
                *(output_annotation+at)= '-';
                at++;
                *(output_annotation+at)= 'k';
                at++;
                *(output_annotation+at)= ' ';
                at++;
                int tempat=0;
                while(1){
                    if((*(*(argv+3)+tempat))=='\0'){
                        break;
                    }
                    *(output_annotation+at) = *(*(argv+3)+tempat);
                    at++;
                    tempat++;
                }
            }
        }
        
        *(output_annotation+at)= '\n';
        at++;
        if(size==0){
            *(output_annotation+at)= '\0';
            at++;
        }else{
            
            at+=annotationcpy(output_annotation+at,input_annotation);
        }

        int tat=at%8;
        if(tat!=0){
            for(int j=0;j<8-tat;j++){
                *(output_annotation+at)= '\0';
                at++;
            }
        }
        hp.data_offset = 24+at;
        size = hp.data_offset - sizeof(AUDIO_HEADER);
        
    }
    
        

    if(global_options & (1UL<<62)){
        unsigned long fac = global_options<<6;
        fac = fac >> 54;
        int factor = fac+1;
        hp.data_size = hp.data_size/factor;
        write_header(&hp);
        write_annotation(output_annotation,size);
     
        int counter= 0;
        while(read_frame(input_frame_int,hp.channels,hp.encoding-1)!=0){
            if(counter%factor ==0){
                write_frame(input_frame_int,hp.channels,hp.encoding-1);
            }
            counter++;
        }
            
        return 1;
    }else if(global_options & (1UL<<61)){
        unsigned long fac = global_options<<6;
        
        fac = fac >> 54;
        int factor = fac+1;
        hp.data_size = hp.data_size*factor;
        write_header(&hp);
        write_annotation(output_annotation,size);
        int counter;
        counter = read_frame(previous_frame_int,hp.channels,hp.encoding-1);
        if(counter!=0){
            write_frame(previous_frame_int,hp.channels,hp.encoding-1);
        }
        while(counter!=0){
            counter = read_frame(input_frame_int,hp.channels,hp.encoding-1);
            if(counter==0){
                break;
            }
            for(int i=1;i<factor;i++){
                calcu_frame(previous_frame_int,input_frame_int,output_frame_int,hp.channels,hp.encoding-1,factor,i);
                write_frame(output_frame_int,hp.channels,hp.encoding-1);
            }
                

            write_frame(input_frame_int,hp.channels,hp.encoding-1);
            framecpy(previous_frame_int,input_frame_int, hp.channels,hp.encoding-1);
        }
          

    }else if(global_options & (1UL<<60)){
        unsigned long tkey = global_options<<32;
        unsigned int key = tkey>>32;
        unsigned int prng;
        unsigned int k;
        mysrand(key);
        write_header(&hp);
        write_annotation(output_annotation,size);
        while(read_frame(input_frame_int,hp.channels,hp.encoding-1)!=0){
       
            for(int g=0;g<hp.channels;g++){
                prng = myrand32();
                k=0;
                for(int j=0;j<hp.encoding-1;j++){
                    k*=256;
                    k+=*(input_frame_int+j+g*(hp.encoding-1));
                }
               
                k = k^prng;

                for(int j =0;j<hp.encoding-1;j++){
                    prng = k << (32-8*(hp.encoding-1)+8*j);
                    prng = prng >>24;
                    *(output_frame_int+j+g*(hp.encoding-1)) = prng;  
                }
            }
            write_frame(output_frame_int,hp.channels,hp.encoding-1);
        }
        return 1;
               
          

    }


    return 1;
}

int read_header(AUDIO_HEADER *hp){
    char c;
    int b;
    int temp;
    for(int j=0;j<4;j++){     //magic number
        c=getchar();
        if(j==0 && (c-'\0')!=46){
            return 0;
        }else if(j==1 && (c-'\0')!=115){
            return 0;
        }else if(j==2 && (c-'\0')!=110){
            return 0;
        }else if(j==3 && (c-'\0')!=100){
            return 0;
        }
        hp->magic_number+=(c-'\0');
        if(j!=3){
            hp->magic_number*=256;
        }
        
    }
    if(hp->magic_number != 779316836){
        return 0;
    }
 

    temp=0;
    for(int j=0;j<4;j++){   //data offset
        temp*=256;
        b=getchar();
        temp+=b;
    }
    if(temp % 8 !=0 || temp>ANNOTATION_MAX ){
        return 0;
    }else{
        hp->data_offset=temp;
    }
    
    temp=0;
    for(int j=0;j<4;j++){  //data size
      
        temp*=256;
     
        b=getchar();
        temp+=(b-'\0');
         
    }
    hp->data_size = temp;
  
    temp=0;
    for(int j=0;j<4;j++){   //encoding
        temp*=256;
        b=getchar();
        temp+=b;
    }
    if(temp==2 || temp==3 || temp==4 || temp==5){
        hp->encoding = temp;
        temp = 0;
    }else{
        return 0;
    }

    temp=0;
    for(int j=0;j<4;j++){   //sample rate
        temp*=256;
        b=getchar();
        temp+=b;
    }
    hp->sample_rate = temp;

    

    temp=0;
    for(int j=0;j<4;j++){   //channels
        temp*=256;
        b=getchar();
        temp+=b;
    }
    if(temp==1 || temp==2){
        hp->channels = temp;
        temp=0;
    }else{
        return 0;
    }

    
    return 1;
}

int write_header(AUDIO_HEADER *hp){
    int a;
    for(int j=0;j<4;j++){
        a = hp->magic_number<<(j*8);
        a=a>>24;
        putchar(a);
    }
    
    
    for(int j=0;j<4;j++){
        a = hp->data_offset<<(j*8);
        a=a>>24;
        putchar(a);
    }

    for(int j=0;j<4;j++){
        a = hp->data_size<<(j*8);
        a=a>>24;
        putchar(a);
    }
    
    for(int j=0;j<4;j++){
        a = hp->encoding<<(j*8);
        a=a>>24;
        putchar(a);
    }

    for(int j=0;j<4;j++){
        a = hp->sample_rate<<(j*8);
        a=a>>24;
        putchar(a);
    }

    for(int j=0;j<4;j++){
        a = hp->channels<<(j*8);
        a=a>>24;
        putchar(a);
    }

    return 1;
}

int read_annotation(char *ap, unsigned int size){
    char c;
    for(int j=0;j<size;j++){
        c = getchar();
        *ap = c;
        ap++;
    }
    if(c!='\0'){
        return 0;
    }

    return 1;
}

int write_annotation(char *ap, unsigned int size){
    char c;
    if(size>ANNOTATION_MAX){
        return 0;
    }
    for(int j=0;j<size;j++){
        c = *ap;
        putchar(c);
        ap++;
    }
    if(c!='\0'){
        return 0;
    }
    return 1;
}

int read_frame(int *fp, int channels, int bytes_per_sample){
    int a;
    int temp=0;
    for(int j=0;j<channels*bytes_per_sample;j++){
        a = getchar();
        if(a == EOF){
            return 0;
        }
        
   
        *(fp+j)=a;
    }
        
    return 1;
}

int write_frame(int *fp, int channels, int bytes_per_sample){
    int c;
    unsigned int temp;
    for(int j=0;j<channels*bytes_per_sample;j++){
        c = *(fp+j);
        
        putchar(c);
        
    }

    return 1;
}

void calcu_frame(int *pp,int *ip,int *op,int channels,int bytes_per_sample,int fac,int k){
    signed int c;
    signed int d;
    signed int e;
    signed int f;


    for(int j=0;j<channels;j++){
        c=0;
        d=0;
        for(int g=0;g<bytes_per_sample;g++){
            c*=256;
            d*=256;
            c+=*(pp+j*bytes_per_sample+g);
            d+=*(ip+j*bytes_per_sample+g);
        }
        e = c+(d-c)*k/fac;
        for(int g=0;g<bytes_per_sample;g++){ 
            f = e <<(32-8*bytes_per_sample+8*g);
            f = f >> 24;
            *(op+j*bytes_per_sample+g) = f;
        }
    }
        
            
        
}


int strToInt(char *str){
    int res = 0;
    
    for(int i=0; *(str+i)!='\0';++i){
        res= res*10+*(str+i)-'0';
    }

    return res;
}

unsigned int strToHex(char *str){
    unsigned int result =0;
    int c;
    while(*str){
        result = result<<4;
        if(c=(*str-'0'),(c>=0 && c<=9)){
            result|=c;
        }else if(c=(*str-'A'),(c>=0 && c<=5)){
            result|=(c+10);
        }else if(c=(*str-'a'),(c>=0 && c<=5)){
            result|=(c+10);
        }else{
            break;
        }
        ++str;
    }
    return result;
}

void framecpy(int *dest,int *src, int channels,int byte_per_sample){
    for(int i=0;i<channels*byte_per_sample;i++){
        *dest = *src;
        dest++;
        src++;
    }
    
}

int annotationcpy(char *dest, char *src){
    int counter=0;
    while(1){
        *dest = *src;
        
        counter++;
        if((*dest)=='\0'){
            break;
        }
        dest++;
        src++;
    }
    return counter;
}
        
        
        

