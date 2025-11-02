#include <errno.h>
#include <stdio.h>
#include "ansii.h"
#include "log.h"
#include <string.h>
#include <unistd.h>

void usage(char* progname){
       fprintf(get_logfile(),"Usage: %s -i INPUT -o OUTPUT",progname);
}

int main(int argc, char** argv){
       int c;
       const char* input = "-",* output = NULL;
       FILE* finput;
       FILE* foutput;

       extern char *optarg;
       extern int optind, optopt;
       while ((c = getopt(argc, argv, "i:o:v")) != -1) {
              switch(c)
              {
              case 'i':
                     input = optarg;
                     break;
              case 'o':
                     output = optarg;
                     break;
              case 'h':
                     usage(*argv); 
                     return 1;
              case 'v':
                     log_level = LOG_VERBOSE;
                     break;
              }
       }
       if(output == NULL){
              log_error("output file not supplied\n");
              return -1;
       }
       // if(input == NULL){
       //        log_error("input file not supplied\n");
       //        return -1;
       // } 
      
       if(!strcmp(input,output)){
              log_error("input and output are the same\n");
              return 1;
       }

       if(!strcmp(input,"-")){
              finput = stdin;
       } else if((finput = fopen(input, "r")) == NULL){
              log_error("input not readable(%s)\n",strerror(errno));
              return errno;
       }

       if(!strcmp(output,"-")){
              foutput = stdout;
       } else if((foutput = fopen(output, "w")) == NULL){
              log_error("output not readable(%s)\n",strerror(errno));
              return errno;
       }

       return ansii_transform(finput,foutput);
}
