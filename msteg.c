/*
msteg - A mini steganography tool
buzzqrd

This is free and unencumbered software released into the public domain.

Anyone is free to copy, modify, publish, use, compile, sell, or
distribute this software, either in source code form or as a compiled
binary, for any purpose, commercial or non-commercial, and by any
means.

In jurisdictions that recognize copyright laws, the author or authors
of this software dedicate any and all copyright interest in the
software to the public domain. We make this dedication for the benefit
of the public at large and to the detriment of our heirs and
successors. We intend this dedication to be an overt act of
relinquishment in perpetuity of all present and future rights to this
software under copyright law.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
IN NO EVENT SHALL THE AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR
OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
OTHER DEALINGS IN THE SOFTWARE.

For more information, please refer to <http://unlicense.org/>

*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#include "defs.h"
#include "argp.h"
#include "bmp.h"


int strcmpb(uint8_t *src, const uint8_t *dst, int size){
	int i=0;
	for(int i=0; i<size; i++){
		if(*(src+i) != *(dst+i)){
			return(1);
		}
	}//for
	return(0);
}



int main(int argc, const char *argv[]){
	
	struct argpc pc;
	argpc_init(argc, &pc);

	FILE *outfp, *infp;

	int pos;
	uint8_t list_stat = FALSE, \
	handle_bin = FALSE, \
	file_read = FALSE, \
	file_write = FALSE, \
	force_stream = FALSE;
	
	char *infile = NULL, \
	*outfile = NULL, \
	*media_filename = NULL, \
	*input_string = NULL;

	/* check argument options */
	if(has_argp(argc, argv, &pc, 0, 4, "t", "-stat", "l", "--list")){
		list_stat = TRUE;
	}
	if( (pos = has_argp(argc, argv, &pc, 1, 2, "i", "-input")) != 0){
		infile = (char *)argv[pos+1];
	}
	if( (pos = has_argp(argc, argv, &pc, 1, 2, "o", "-output")) != 0){
		outfile = (char *)argv[pos+1];
	}
	/*if( (pos = has_argp(argc, argv, &pc, 0, 5, "b", "-bin", "-binary", "f", "-file")) != 0){
		handle_bin = TRUE;
	}*/
	if( (pos = has_argp(argc, argv, &pc, 1, 2, "s", "-string")) != 0){
		input_string = (char *)argv[pos+1];
	}
	if( (pos = has_argp(argc, argv, &pc, 0, 2, "r", "-read")) != 0){
		file_read = TRUE;
	}
	if( (pos = has_argp(argc, argv, &pc, 0, 2, "w", "-write")) != 0){
		file_write = TRUE;
	}
	if( (pos = has_argp(argc, argv, &pc, 0, 1, "-stream")) != 0){
		force_stream = TRUE;
	}


	/* check for invalid arguments*/
	if(pos = inval_argp(argc, argv, &pc)){
		fprintf(stderr, "Error: Invalid argument at position %d.\n", pos);
		return(-1);
	}

	if(!unuse_argp(argc, argv, &pc)){
		fprintf(stderr, "Error: No free arguments. Please provide a media file container.\n");
		return(-1);
	}

	if(pc.free){
		/* first free argument should be the media filename */
		media_filename = pc.args[0];
	}
	else{
		fprintf(stderr,"Error: Could not find any specified media file.\n");
	}

	

	if(infile){
		if(handle_bin){
			infp = fopen(infile, "rb");
		}
		else{
			infp = fopen(infile, "r");
		}
	}
	else{
		infp = stdin;
	}

	if(outfile){
		if(handle_bin){ 
			outfp = fopen(outfile, "wb");
		}
		else{
			outfp = fopen(outfile, "w");
		}
	}
	else{
		outfp = stdout;
	}


	/* make sure files opened */
	if(outfp == NULL || infp == NULL){
		fprintf(stderr, "Error: Could not open I/O file.\n");
		argpc_close(&pc);
		if(outfp != stdout && outfp != NULL){fclose(outfp);}
		if(infp != stdin && infp != NULL){fclose(infp);}
		return(-1);
	}



	FILE *fp = fopen(media_filename, "r+b");

	if(fp == NULL){
		fprintf(stderr, "Error: Could not open media file.\n");
		argpc_close(&pc);
		if(outfp != stdout && outfp != NULL){fclose(outfp);}
		if(infp != stdin && infp != NULL){fclose(infp);}
		return(-1);
	}



	/* TODO: update this filetype detection */
	uint8_t chunk[CHUNK_SIZE];
	memset(chunk, 0, CHUNK_SIZE);

	fread(chunk, 1, 8, fp);

	if(strcmpb(chunk, "BM", 2)){
		fprintf(stderr, "Error: This is not a BMP file.\n");
		argpc_close(&pc);
		fclose(fp);
		return(-1);
	}	



	if(list_stat){
		print_bmp_stats(fp);
	}
	else if(file_read){
		bmp_read(fp, outfp);
	}
	else if(file_write){
		bmp_write(fp, infp, input_string, force_stream);
	}
	else{
		fprintf(stderr, "No actions specified. Nothing to do.\n");
	}
	
	fclose(fp);
	argpc_close(&pc);
	return(0);
}





