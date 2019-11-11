#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <getopt.h>
#include <string.h>
#include <readline/readline.h>
#include <readline/history.h>
#include <sys/time.h>
#include <time.h>
#include <math.h>
#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>
#include <sys/types.h>

#include "imprimer.h"

/*
 * "Imprimer" printer spooler.
 */

static const char * const cmdPrompt = "imp>";
static const char * const help = "help";
static const char * const quit = "quit";
static const char * const type = "type";
static const char * const printer = "printer";
static const char * const conversion = "conversion";
static const char * const printerss = "printers";
static const char * const jobss = "jobs";
static const char * const print = "print";
static const char * const cancel = "cancel";
static const char * const pauses = "pause";
static const char * const resume = "resume";
static const char * const disable = "disable";
static const char * const enable = "enable";

#define MAX_JOBS 1024
#define _GNU_SOURCE


void handler(int sig){
    pid_t pid;
    if(sig==SIGCHLD){
        pid = waitpid(-1,NULL,WNOHANG);
        printf("Child %d ended\n",pid);
    }
}



int main(int argc, char *argv[])
{
    signal(SIGCHLD,handler);

    char optval;
    char *input = NULL;
    char *cmd = (char *)malloc(20);

    char *errormsg = (char *)malloc(100);
    char *printermsg = (char *)malloc(100);
    char *jobmsg = (char *)malloc(100);


    PRINTER printers[MAX_PRINTERS];
    JOB jobs[MAX_JOBS];

    int jobcount=0;
    //int jobnum=0;
    int printercount = 0;
    char *types[100];
    int typecount = 0;
    char *conv_name[100];
    char *conv_src[100];
    char *conv_dest[100];
    int convcount = 0;

    int optioni = 0;
    int optiono = 0;


    if(optind<argc){
        if(((optval = getopt(argc, argv, "io")) != -1)) {
            switch(optval) {
            case 105:
                optioni=1;
                break;
            case 111:
                optiono=1;
                break;
            case '?':
                fprintf(stderr, "Usage: %s [-i <cmd_file>] [-o <out_file>]\n", argv[0]);
                exit(EXIT_FAILURE);
                break;
            default:
                break;
            }

        }else{
            fprintf(stderr, "Usage: %s [-i <cmd_file>] [-o <out_file>]\n", argv[0]);
            exit(EXIT_FAILURE);
        }
    }
    if(argc==5){
        if(((optval = getopt(argc, argv, "o")) != -1)){
            optiono=1;
        }
    }

    if(optioni==1){
        FILE * fp;
        char * line = (char *)malloc(100);


        fp = fopen(argv[2], "r");
        if (fp == NULL)
            exit(EXIT_FAILURE);


        while ((fgets(line,100,fp))!=NULL) {

                    input = line;
                    char *pos;
                    if((pos=strchr(line,'\n'))!=NULL){
                        *pos = '\0';
                    }
                    int o=0;
                    char *p = strtok(input," ");
                    char *array[40];

                    while(p!=NULL){
                        array[o++]=p;
                        p = strtok(NULL," ");
                    }
                    int h=o;

                    if(h>=1){
                        cmd = array[0];
                    }

                    if(h==0){
                        continue;
                    }
                    else if(strcmp(cmd,help)==0 && h==1){
                         fprintf(stdout, "USAGE: %s \n", "imprimer\n" \
                        "Miscellaneous commands:\n" \
                        "    help    Lists all of the types of commands\n" \
                        "    quit    Causes execution to terminate\n" \
                        "Configuration commands:\n" \
                        "    type file_type    Declares file_type to be a file type to be supported by the program\n" \
                        "    printer printer_name file_type    Declares the existence of a printer named printer_name,which is capable of printing files of type file_type\n" \
                        "    conversion file_type1 file_type2 conversion_program [arg1 arg2 ...]    Declares that files of type file_type1 can be converted in to file_type2 by running program conversion_program with any arguments that have been indicated\n" \
                        "Informational commands:\n" \
                        "    printers    Prints a report on the current status of the declared printers\n" \
                        "    jobs    Prints a similar status report for the print jobs that have been queued\n" \
                        "Spooling commands:\n" \
                        "    print file_name [printer1 printer2 ...]    Sets up a job for printing file_name\n" \
                        "    cancel job_number    Cancels an existing job\n" \
                        "    pause job_number    Pauses a job that is currently being processed\n" \
                        "    resume job_number    Resumes a job that was previously paused\n" \
                        "    disable printer_name    Sets the state of a specified printer to disabled\n" \
                        "    enable printer_name    Sets the state of a specified print to enabled\n");
                    }
                    else if(strcmp(cmd,quit)==0 && h==1){
                        exit(EXIT_SUCCESS);
                    }
                    else if(strcmp(cmd,type)==0 && h==2){
                        int typeflag=0;

                        for(int i=0;i<typecount;i++){
                            if(strcmp(types[i],array[1])==0){
                                fprintf(stdout,"%s",imp_format_error_message("Duplicate type\n",errormsg,100));
                                typeflag=1;
                                break;
                            }
                        }

                        if(typeflag==0){
                            char *typetrans = (char*)malloc(100);
                            strcpy(typetrans,array[1]);
                            types[typecount] = typetrans;
                            typecount++;
                        }

                    }
                    else if(strcmp(cmd,printer)==0 && h==3){
                        if(printercount>=32){
                            fprintf(stdout,"%s",imp_format_error_message("Exceed the maximum number of printers\n",errormsg,100));
                        }
                        int nameflag=0;
                        for(int i=0;i<printercount;i++){
                            if(strcmp(printers[i].name,array[1])==0){
                                fprintf(stdout,"%s",imp_format_error_message("Invalid printer name\n",errormsg,100));
                                nameflag=1;
                                break;
                            }
                        }
                        if(nameflag==1){
                            continue;
                        }
                        int typeflag=0;
                        for(int i=0;i<typecount;i++){
                            if(strcmp(types[i],array[2])==0){
                                typeflag=1;
                                break;
                            }
                        }
                        if(typeflag==0){
                            fprintf(stdout,"%s",imp_format_error_message("Invalid file type\n",errormsg,100));
                            continue;
                        }

                        printers[printercount].id=printercount;
                        printers[printercount].enabled = 0;
                        printers[printercount].busy = 0;
                        char *typetrans = (char*)malloc(100);
                        strcpy(typetrans,array[1]);
                        printers[printercount].name = typetrans;
                        typetrans = (char*)malloc(100);
                        strcpy(typetrans,array[2]);
                        printers[printercount].type = typetrans;
                        imp_connect_to_printer(&printers[printercount],PRINTER_NORMAL);

                        fprintf(stdout,"%s\n",imp_format_printer_status(&printers[printercount],printermsg,100));
                        printercount++;


                    }
                    else if(strcmp(cmd,conversion)==0 && h>=4){
                        int convtypeflag1=0;
                        int convtypeflag2=0;
                        for(int i=0;i<typecount;i++){
                            if(strcmp(types[i],array[2])==0){
                                convtypeflag1=1;
                                break;
                            }
                        }
                        for(int i=0;i<typecount;i++){
                            if(strcmp(types[i],array[1])==0){
                                convtypeflag2=1;
                                break;
                            }
                        }
                        if(convtypeflag1==0 || convtypeflag2==0){
                            fprintf(stdout,"%s",imp_format_error_message("Invalid target file type\n",errormsg,100));
                        }
                        if(h==4){
                            int convflag=0;
                            for(int i =0;i<convcount;i++){
                                if(strcmp(conv_name[i],array[3])==0){
                                    convflag=1;
                                    break;
                                }
                                if(strcmp(conv_src[i],array[1])==0 && strcmp(conv_dest[i],array[2])==0){
                                    convflag=1;
                                    break;
                                }
                            }
                            if(convflag==1){
                                fprintf(stdout,"%s",imp_format_error_message("Conversion program already exists\n",errormsg,100));
                                continue;
                            }
                            char *typetrans = (char*)malloc(100);
                            strcpy(typetrans,array[3]);
                            conv_name[convcount] = typetrans;
                            typetrans = (char*)malloc(100);
                            strcpy(typetrans,array[1]);
                            conv_src[convcount] = typetrans;
                            typetrans = (char*)malloc(100);
                            strcpy(typetrans,array[2]);
                            conv_dest[convcount] = typetrans;
                            convcount++;
                        }else{

                            char *conversion_name = (char *)malloc(100);
                            strcpy(conversion_name ,array[3]);
                            for(int i=0;i<h-4;i++){
                                strcat(conversion_name," ");
                                strcat(conversion_name,array[4+i]);
                            }
                            int convflag = 0;
                            for(int i=0;i<convcount;i++){
                                if(strcmp(conv_name[i],conversion_name)==0){
                                    convflag=1;
                                    break;
                                }
                                if(strcmp(conv_src[i],array[1])==0 && strcmp(conv_dest[i],array[2])==0){
                                    convflag=1;
                                    break;
                                }
                            }
                            if(convflag==1){
                                fprintf(stdout,"%s",imp_format_error_message("Conversion program already exists\n",errormsg,100));
                                continue;
                            }
                            conv_name[convcount] = conversion_name;
                            char *typetrans = (char*)malloc(100);
                            strcpy(typetrans,array[1]);
                            conv_src[convcount] = typetrans;
                            typetrans = (char*)malloc(100);
                            strcpy(typetrans,array[2]);
                            conv_dest[convcount] = typetrans;
                            convcount++;

                        }


                    }
                    else if(strcmp(cmd,printerss)==0 && h==1){
                        for(int i=0;i<printercount;i++){
                            fprintf(stdout,"%s\n",imp_format_printer_status(&printers[i],printermsg,100));
                        }
                    }
                    else if(strcmp(cmd,jobss)==0 && h==1){
                        for(int i=0;i<jobcount;i++){
                            fprintf(stdout,"%s\n",imp_format_job_status(&jobs[i],jobmsg,100));
                        }
                    }
                    else if(strcmp(cmd,print)==0 && h>=2){
                        if(h==2){
                            char *fsfs = (char*)malloc(100);
                            strcpy(fsfs,array[1]);
                            int q=0;
                            char *j = strtok(array[1],".");
                            char *jobarray[10];

                            while(j!=NULL){
                                jobarray[q++]=j;
                                j = strtok(NULL,".");
                            }
                            int jobtypeflag=0;
                            for(int i=0;i<typecount;i++){
                                if(strcmp(jobarray[q-1],types[i])==0){
                                    jobtypeflag=1;
                                    break;
                                }
                            }
                            if(jobtypeflag==0){
                                fprintf(stdout,"%s",imp_format_error_message("Unsupported file type\n",errormsg,100));
                                continue;
                            }

                            jobs[jobcount].jobid = jobcount;
                            jobs[jobcount].status = QUEUED;
                            char *typetrans = (char*)malloc(100);
                            strcpy(typetrans,jobarray[q-1]);
                            jobs[jobcount].file_type = typetrans;
                            jobs[jobcount].eligible_printers = ANY_PRINTER;
                            jobs[jobcount].file_name=fsfs;


                            gettimeofday(&jobs[jobcount].creation_time,NULL);
                            gettimeofday(&jobs[jobcount].change_time,NULL);
                            jobcount++;

                        }else{
                            uint32_t temp = 0;
                            int jobprintercount=0;
                            for(int i=2;i<h;i++){
                                for(int z=0;z<printercount;z++){
                                    if(strcmp(printers[z].name,array[i])==0){
                                        temp+=pow(2,printers[z].id);
                                        jobprintercount++;
                                        break;
                                    }
                                }
                            }

                            if(jobprintercount!=h-2){
                                fprintf(stdout,"%s",imp_format_error_message("Invalid printer\n",errormsg,100));
                                continue;
                            }
                            char *fsfs = (char*)malloc(100);
                            strcpy(fsfs,array[1]);
                            int q=0;
                            char *j = strtok(array[1],".");
                            char *jobarray[10];

                            while(j!=NULL){
                                jobarray[q++]=j;
                                j = strtok(NULL,".");
                            }
                            int jobtypeflag=0;
                            for(int i=0;i<typecount;i++){
                                if(strcmp(jobarray[q-1],types[i])==0){
                                    jobtypeflag=1;
                                    break;
                                }
                            }
                            if(jobtypeflag==0){
                                errormsg = imp_format_error_message("Unsupported file type\n",errormsg,100);
                                fprintf(stdout,"%s",errormsg);
                                continue;
                            }

                            jobs[jobcount].jobid = jobcount;
                            jobs[jobcount].status = QUEUED;
                            char *typetrans = (char*)malloc(100);
                            strcpy(typetrans,jobarray[q-1]);
                            jobs[jobcount].file_type = typetrans;
                            jobs[jobcount].eligible_printers = temp;
                            jobs[jobcount].file_name=fsfs;


                            gettimeofday(&jobs[jobcount].creation_time,NULL);
                            gettimeofday(&jobs[jobcount].change_time,NULL);
                            jobcount++;
                        }
                    }
                    else if(strcmp(cmd,cancel)==0 && h==2){
                        break;
                    }
                    else if(strcmp(cmd,pauses)==0 && h==2){
                        break;
                    }
                    else if(strcmp(cmd,resume)==0 && h==2){
                        break;
                    }
                    else if(strcmp(cmd,disable)==0 && h==2){
                        break;
                    }
                    else if(strcmp(cmd,enable)==0 && h==2){
                        break;
                    }
                    else{
                        errormsg = imp_format_error_message("Invalid command, enter \'help\' to get more infomation\n",errormsg,100);
                        fprintf(stdout,"%s",errormsg);
                    }

        }

        fclose(fp);
        if (line)
            free(line);
    }

    if(optiono==1){
        if(optioni==1){
            freopen(argv[4],"w+",stdout);
        }
        else{
            freopen(argv[2],"w+",stdout);
        }
    }


    while(1){
        FILE *fp;
        fp = fopen("bin/reallyweirdsolutionnvm","w");
        input = readline(cmdPrompt);
        int o=0;
        char *p = strtok(input," ");
        char *array[40];

        while(p!=NULL){
            array[o++]=p;
            p = strtok(NULL," ");
        }
        int h=o;

        if(h>=1){
            cmd = array[0];
        }

        if(h==0){
            continue;
        }
        else if(strcmp(cmd,help)==0 && h==1){
             fprintf(stdout, "USAGE: %s \n", "imprimer\n" \
            "Miscellaneous commands:\n" \
            "    help    Lists all of the types of commands\n" \
            "    quit    Causes execution to terminate\n" \
            "Configuration commands:\n" \
            "    type file_type    Declares file_type to be a file type to be supported by the program\n" \
            "    printer printer_name file_type    Declares the existence of a printer named printer_name,which is capable of printing files of type file_type\n" \
            "    conversion file_type1 file_type2 conversion_program [arg1 arg2 ...]    Declares that files of type file_type1 can be converted in to file_type2 by running program conversion_program with any arguments that have been indicated\n" \
            "Informational commands:\n" \
            "    printers    Prints a report on the current status of the declared printers\n" \
            "    jobs    Prints a similar status report for the print jobs that have been queued\n" \
            "Spooling commands:\n" \
            "    print file_name [printer1 printer2 ...]    Sets up a job for printing file_name\n" \
            "    cancel job_number    Cancels an existing job\n" \
            "    pause job_number    Pauses a job that is currently being processed\n" \
            "    resume job_number    Resumes a job that was previously paused\n" \
            "    disable printer_name    Sets the state of a specified printer to disabled\n" \
            "    enable printer_name    Sets the state of a specified print to enabled\n");
        }
        else if(strcmp(cmd,quit)==0 && h==1){
            break;
        }
        else if(strcmp(cmd,type)==0 && h==2){
            int typeflag=0;
            for(int i=0;i<typecount;i++){
                if(strcmp(types[i],array[1])==0){
                    fprintf(stdout,"%s",imp_format_error_message("Duplicate type\n",errormsg,100));
                    typeflag=1;
                    break;
                }
            }
            if(typeflag==0){
                char *typetrans = (char*)malloc(100);
                strcpy(typetrans,array[1]);
                types[typecount] = typetrans;
                typecount++;
            }
        }
        else if(strcmp(cmd,printer)==0 && h==3){
            if(printercount>=32){
                fprintf(stdout,"%s",imp_format_error_message("Exceed the maximum number of printers\n",errormsg,100));
            }
            int nameflag=0;
            for(int i=0;i<printercount;i++){
                if(strcmp(printers[i].name,array[1])==0){
                    fprintf(stdout,"%s",imp_format_error_message("Invalid printer name\n",errormsg,100));
                    nameflag=1;
                    break;
                }
            }
            if(nameflag==1){
                continue;
            }
            int typeflag=0;
            for(int i=0;i<typecount;i++){
                if(strcmp(types[i],array[2])==0){
                    typeflag=1;
                    break;
                }
            }
            if(typeflag==0){
                fprintf(stdout,"%s",imp_format_error_message("Invalid file type\n",errormsg,100));
                continue;
            }

            printers[printercount].id=printercount;
            printers[printercount].enabled = 0;
            printers[printercount].busy = 0;
            char *typetrans = (char*)malloc(100);
            strcpy(typetrans,array[1]);
            printers[printercount].name = typetrans;
            typetrans = (char*)malloc(100);
            strcpy(typetrans,array[2]);
            printers[printercount].type = typetrans;
            imp_connect_to_printer(&printers[printercount],PRINTER_NORMAL);
            fprintf(stdout,"%s\n",imp_format_printer_status(&printers[printercount],printermsg,100));
            printercount++;


        }
        else if(strcmp(cmd,conversion)==0 && h>=4){
            int convtypeflag1=0;
            int convtypeflag2=0;
            for(int i=0;i<typecount;i++){
                if(strcmp(types[i],array[2])==0){
                    convtypeflag1=1;
                    break;
                }
            }
            for(int i=0;i<typecount;i++){
                if(strcmp(types[i],array[1])==0){
                    convtypeflag2=1;
                    break;
                }
            }
            if(convtypeflag1==0 || convtypeflag2==0){
                fprintf(stdout,"%s",imp_format_error_message("Invalid target file type\n",errormsg,100));
            }
            if(h==4){
                int convflag=0;
                for(int i =0;i<convcount;i++){
                    if(strcmp(conv_name[i],array[3])==0){
                        convflag=1;
                        break;
                    }
                    if(strcmp(conv_src[i],array[1])==0 && strcmp(conv_dest[i],array[2])==0){
                        convflag=1;
                        break;
                    }
                }
                if(convflag==1){
                    fprintf(stdout,"%s",imp_format_error_message("Conversion program already exists\n",errormsg,100));
                    continue;
                }
                char *typetrans = (char*)malloc(100);
                strcpy(typetrans,array[3]);
                conv_name[convcount] = typetrans;
                typetrans = (char*)malloc(100);
                strcpy(typetrans,array[1]);
                conv_src[convcount] = typetrans;
                typetrans = (char*)malloc(100);
                strcpy(typetrans,array[2]);
                conv_dest[convcount] = typetrans;
                convcount++;
            }else{
                char *conversion_name = (char *)malloc(100);
                strcpy(conversion_name ,array[3]);
                for(int i=0;i<h-4;i++){
                    strcat(conversion_name," ");
                    strcat(conversion_name,array[4+i]);
                }
                int convflag = 0;
                for(int i=0;i<convcount;i++){
                    if(strcmp(conv_name[i],conversion_name)==0){
                        convflag=1;
                        break;
                    }
                    if(strcmp(conv_src[i],array[1])==0 && strcmp(conv_dest[i],array[2])==0){
                        convflag=1;
                        break;
                    }

                }
                if(convflag==1){
                    fprintf(stdout,"%s",imp_format_error_message("Conversion program already exists\n",errormsg,100));
                    continue;
                }
                conv_name[convcount] = conversion_name;
                char *typetrans = (char*)malloc(100);
                strcpy(typetrans,array[1]);
                conv_src[convcount] = typetrans;
                typetrans = (char*)malloc(100);
                strcpy(typetrans,array[2]);
                conv_dest[convcount] = typetrans;
                convcount++;
            }

        }
        else if(strcmp(cmd,printerss)==0 && h==1){
            for(int i=0;i<printercount;i++){
                fprintf(stdout,"%s\n",imp_format_printer_status(&printers[i],printermsg,100));
            }
        }
        else if(strcmp(cmd,jobss)==0 && h==1){
            for(int i=0;i<jobcount;i++){
                fprintf(stdout,"%s\n",imp_format_job_status(&jobs[i],jobmsg,100));
            }
        }
        else if(strcmp(cmd,print)==0 && h>=2){
            if(h==2){
                char *fsfs = (char*)malloc(100);
                strcpy(fsfs,array[1]);
                int q=0;
                char *j = strtok(array[1],".");
                char *jobarray[10];

                while(j!=NULL){
                    jobarray[q++]=j;
                    j = strtok(NULL,".");
                }
                int jobtypeflag=0;
                for(int i=0;i<typecount;i++){
                    if(strcmp(jobarray[q-1],types[i])==0){
                        jobtypeflag=1;
                        break;
                    }
                }
                if(jobtypeflag==0){
                    fprintf(stdout,"%s",imp_format_error_message("Unsupported file type\n",errormsg,100));
                    continue;
                }

                jobs[jobcount].jobid = jobcount;
                jobs[jobcount].status = QUEUED;
                char *typetrans = (char*)malloc(100);
                strcpy(typetrans,jobarray[q-1]);
                jobs[jobcount].file_type = typetrans;
                jobs[jobcount].eligible_printers = ANY_PRINTER;
                jobs[jobcount].file_name=fsfs;


                gettimeofday(&jobs[jobcount].creation_time,NULL);
                gettimeofday(&jobs[jobcount].change_time,NULL);
                fprintf(stdout,"%s\n",imp_format_job_status(&jobs[jobcount],jobmsg,100));
                jobcount++;

            }else{
                uint32_t temp = 0;
                int jobprintercount=0;
                for(int i=2;i<h;i++){
                    for(int z=0;z<printercount;z++){
                        if(strcmp(printers[z].name,array[i])==0){
                            temp+=pow(2,printers[z].id);
                            jobprintercount++;
                            break;
                        }
                    }
                }

                if(jobprintercount!=h-2){
                    fprintf(stdout,"%s",imp_format_error_message("Invalid printer\n",errormsg,100));
                    continue;
                }
                char *fsfs = (char*)malloc(100);
                strcpy(fsfs,array[1]);
                int q=0;
                char *j = strtok(array[1],".");
                char *jobarray[10];

                while(j!=NULL){
                    jobarray[q++]=j;
                    j = strtok(NULL,".");
                }
                int jobtypeflag=0;
                for(int i=0;i<typecount;i++){
                    if(strcmp(jobarray[q-1],types[i])==0){
                        jobtypeflag=1;
                        break;
                    }
                }
                if(jobtypeflag==0){
                    errormsg = imp_format_error_message("Unsupported file type\n",errormsg,100);
                    fprintf(stdout,"%s",errormsg);
                    continue;
                }

                jobs[jobcount].jobid = jobcount;
                jobs[jobcount].status = QUEUED;
                char *typetrans = (char*)malloc(100);
                strcpy(typetrans,jobarray[q-1]);
                jobs[jobcount].file_type = typetrans;
                jobs[jobcount].eligible_printers = temp;
                jobs[jobcount].file_name=fsfs;


                gettimeofday(&jobs[jobcount].creation_time,NULL);
                gettimeofday(&jobs[jobcount].change_time,NULL);
                fprintf(stdout,"%s\n",imp_format_job_status(&jobs[jobcount],jobmsg,100));
                jobcount++;
            }
        }
        else if(strcmp(cmd,cancel)==0 && h==2){
            for(int i=0;i<jobcount;i++){
                if(strcmp(jobs[i].file_name,array[1])==0){
                    if(jobs[i].status == QUEUED){
                        jobs[i].status = ABORTED;
                        gettimeofday(&jobs[jobcount].change_time,NULL);
                    }else{
                        //RUNNING OR PAUSED
                    }
                }
            }
        }
        else if(strcmp(cmd,pauses)==0 && h==2){
            break;
        }
        else if(strcmp(cmd,resume)==0 && h==2){
            break;
        }
        else if(strcmp(cmd,disable)==0 && h==2){
            int validprinter=-1;
            for(int i=0;i<printercount;i++){
                if(strcmp(printers[i].name,array[1])==0){
                    if(printers[i].enabled==0){
                        errormsg = imp_format_error_message("Printer already disabled.",errormsg,100);
                        fprintf(stdout,"%s",errormsg);
                        break;
                    }
                    printers[i].enabled=0;
                    validprinter = 1;
                    fprintf(stdout,"%s\n",imp_format_printer_status(&printers[i],printermsg,100));

                }
            }
            if(validprinter ==-1){
                errormsg = imp_format_error_message("Invalid printer name\n",errormsg,100);
                fprintf(stdout,"%s",errormsg);
            }
        }
        else if(strcmp(cmd,enable)==0 && h==2){
            int validprinter=-1;
            for(int i=0;i<printercount;i++){
                if(strcmp(printers[i].name,array[1])==0){
                    if(printers[i].enabled==1){
                        errormsg = imp_format_error_message("Printer already enabled.",errormsg,100);
                        fprintf(stdout,"%s",errormsg);
                        break;
                    }
                    printers[i].enabled=1;
                    validprinter = i;
                    fprintf(stdout,"%s\n",imp_format_printer_status(&printers[i],printermsg,100));

                }
            }

            if(validprinter ==-1){
                errormsg = imp_format_error_message("Invalid printer name\n",errormsg,100);
                fprintf(stdout,"%s",errormsg);
            }
        }
        else{
            errormsg = imp_format_error_message("Invalid command, enter \'help\' to get more infomation\n",errormsg,100);
            fprintf(stdout,"%s",errormsg);
        }


        for(int i=0;i<jobcount;i++){
            for(int j=0;j<printercount;j++){
                if(strcmp(jobs[i].file_type,printers[j].type)==0){
                    if((jobs[i].status==QUEUED) && (printers[j].busy==0) && (printers[j].enabled==1)){
                        uint32_t printertest=pow(2,printers[j].id);
                        if((jobs[i].eligible_printers&printertest)!=0){
                            jobs[i].status=RUNNING;
                            printers[j].busy=1;
                            jobs[i].chosen_printer = &printers[j];
                            gettimeofday(&jobs[i].change_time,NULL);
                            fprintf(stdout,"%s\n",imp_format_printer_status(&printers[i],printermsg,100));
                            fprintf(stdout,"%s\n",imp_format_job_status(&jobs[i],jobmsg,100));


                            break;
                        }
                    }
                }
            }
        }
        fclose(fp);
    }
    remove("bin/reallyweirdsolutionnvm");
    exit(EXIT_SUCCESS);
}

