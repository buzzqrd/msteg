/*
argp.h Simple functions for finding arument parameters
buzzqrd
*/
#ifndef _ARGP_H
#define _ARGP_H

#include <stdlib.h>
#include <stdarg.h>
#include <string.h>


struct argpc {
	int *len;
	int free;
	char **args;
};


void argpc_init(int argc, struct argpc *pc){
	pc->free = 0;
	pc->len = (int *)malloc(argc * sizeof(int));
	pc->args = (char **)malloc(argc * sizeof(char *));
	return;
}

void argpc_close(struct argpc *pc){
	free(pc->len);
	return;
}

int charg_comp(const char* arg, char c){
	char *a = (char *)arg;
	if(*a != '-'){
		return(0);
	}
	a++;
	while(*a && *a != '-'){
		if(*a == c){
			return(1);
		}
		a++;
	}
	return(0);
}

int resolve_arg(int argc, const char *argv[], struct argpc *pc, char *str){
	int i;

	if(*(str+1)){
		for(i=1; i<argc; i++){
			if( !strcmp(argv[i]+1, str) ){
				//pc->len[i] = 1;
				return(i);
			}
		}
		return(0);	
	}
		
	for(i=1; i<argc; i++){
		if( charg_comp(argv[i], *str) ){
			//pc->len[i]++;
			return(i);
		}
	}

	return(0);
}

/* returns 0 if none of the given arguments are specified in the command line args.
otherwise, returns the argv index of where the argument was found
argc: the number of arguments on the command line. same as argc passed in to main.
argv: The command line arguments given to the program's main function.
pc: pointer to the shared argpc struct.
reserve: number of arguments after this one that should be reserved, not considered args, or put in pc.args
argn: the number of following arguments passed in.
...: any number of aruments that should all be considered the same. for instance, -o -O, and --output can 
all be considered the same.
*/
int has_argp(int argc, const char *argv[], struct argpc *pc, int reserve, int argn, ...){
	va_list ap;
	char *p;

	va_start(ap, argn);
	
	int i, k, a;
	for(i=0; i<argn; i++){
		p = va_arg(ap, char *);
		if( (a = resolve_arg(argc, argv, pc, p)) ){
			pc->len[a]++;
			for(k=0; k<reserve; k++){
				pc->len[a+k+1] = -1;
			}
			return(a);
		}
	}

	va_end(ap);
	return(0);
}

/* returns index of any invalid/unused/duplicate argument parameters, 0 if all are valid*/
int inval_argp(int argc, const char *argv[], struct argpc *pc){
	int i, lc, lf=0;
	char *s;
	for(i=1; i<argc; i++){
		if(pc->len[i] == -1){
			continue;
		}
		if(*argv[i] != '-'){
			continue;
		}
		if(*(argv[i]+1) == '-' && pc->len[i] == 1){
			continue;
		}
		
		s = (char *)argv[i]+1;
		lc = 0;
		while(*s != 0){
			lc++;
			s++;
		}
		if(lc != pc->len[i]){
			return(i);
		}
	}

	return(0);
}


int unuse_argp(int argc, const char *argv[], struct argpc *pc){
	int i, cnt=0;
	for(i=1; i<argc; i++){
		if(pc->len[i] == 0){
			pc->args[cnt] = (char *)argv[i];
			cnt++;
		}
	}
	pc->free = cnt;
	return(cnt);
}



int debug_argpc(int argc, struct argpc *pc){
	int i;
	fprintf(stderr, "args: ");
	for(i=0; i<argc; i++){
		fprintf(stderr, "[%d]",pc->len[i]);
	}
	fprintf(stderr, "\n");
	return(0);
}


#endif
