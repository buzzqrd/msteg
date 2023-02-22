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

//#include <arpa/inet.h>
#include "argp.h"

#define TRUE 1
#define FALSE 0

#define CHUNK_SIZE 32
#define FIELD_SIZE 4

#define STR_TAG 0x01
#define BIN_TAG 0x02

#define FILE_WRITE_CHUNK_SIZE 512

int strcmpb(uint8_t *src, const uint8_t *dst, int size){
	int i=0;
	for(int i=0; i<size; i++){
		if(*(src+i) != *(dst+i)){
			return(1);
		}
	}//for
	return(0);
}

int get_bmp_data_start(FILE *fp){
	fseek(fp, 0x000A, SEEK_SET);
	int start;
	fread(&start, FIELD_SIZE, 1, fp);
	return(start);
}

int get_max_data_size(FILE *fp){
	fseek(fp, 0x0002, SEEK_SET);
	int dat;
	fread(&dat, sizeof(dat), 1, fp);
	printf("File Size: %d\n", dat);
	int file_size = dat;	
	int data_start = get_bmp_data_start(fp);
	return((file_size-data_start-2)/8);
}



int bmp_data_set(uint8_t c, int i, FILE *fp){
	int start = get_bmp_data_start(fp);
	int byte_index = (i*8) + start;
	fseek(fp, byte_index, SEEK_SET);	
	uint8_t b;
	for(i=0; i<8; i++){
		fread(&b, 1, 1, fp);
		fseek(fp, -1, SEEK_CUR);
		b = (b & ~1) | ((c >> i)  & 1 );
		fwrite(&b, 1, 1, fp);
	}
	return(0);
}

uint8_t bmp_data_get(int i, FILE *fp){
	uint8_t b=0, c=0;
	int start = get_bmp_data_start(fp);
	int byte_index = (i*8) + start;
	fseek(fp, byte_index, SEEK_SET); 
	for(i=0; i<8; i++){
		fread(&b, 1, 1, fp);
		c = (c >> 1) | ((b & 1 ) << 7);
	}
	
	return(c);
}




int print_bmp_stats(FILE *fp){
	fseek(fp, 0x0002, SEEK_SET);
	int dat;
	fread(&dat, sizeof(dat), 1, fp);
	printf("File Size: %d\n", dat);
	int file_size = dat;	
	
	fseek(fp, 0x0012, SEEK_SET);
	fread(&dat, sizeof(dat), 1, fp);
	printf("Image width: %d\n", dat);
	fread(&dat, sizeof(dat), 1, fp);
	printf("Image height: %d\n", dat);

	fseek(fp, 0x001E, SEEK_SET);
	fread(&dat, sizeof(dat), 1, fp);
	printf("Compression type: %d ", dat);
	switch(dat){
		case 0:
			printf("(BI_RGB)\n");
			break;
		case 1: 
			printf("(8-bit BI_RLE8 encoding)\n");
			break;
		case 2: 
			printf("(4-bit BI_RLE4 encoding)\n");
			break;
		default:
			printf("(Unknown compression type.)\n");
	}
	printf("Compatable?: %s\n", (!dat ? "yes" : "no") );
	
	int data_start = get_bmp_data_start(fp);
	printf("Max amount of hidden data: %d bytes\n", ((file_size-data_start-1)/8)-5);

	return(0);
}

int bmp_write_string(FILE *media, char *str){
	
	if(strlen(str) >= get_max_data_size(media)){
		fprintf(stderr, "This data is too large to store in this media file.\n");
		return(-1);
	}

	/* add the tag for a string */
	bmp_data_set(STR_TAG, 0, media);	

	int i=1;
	uint8_t b, c;
	char *s = str;
	while(*s != 0){
		bmp_data_set(*s, i, media);
		i++;
		s++;
	}
	bmp_data_set(0, i, media);

	return(0);
}


int bmp_read_string(FILE *media, FILE *out){
	int i=1;
	uint8_t b, c=0;
	do{
		c = bmp_data_get(i, media);
		fprintf(out, "%c", c);
		i++;
	}while(c != 0);
	printf("\n");

	return(0);
}


int bmp_write_file(FILE *media, FILE *src){
	int max = get_max_data_size(media)-4;  
	uint8_t datbuf[FILE_WRITE_CHUNK_SIZE];
	int data_size = 0, i=0, index=5;

	/* embed the file tag */
	bmp_data_set(BIN_TAG, 0, media);	


	/* embed the file size (4-bytes) */
	fseek(src, 0, SEEK_END);
	unsigned int file_size = ftell(src);
	rewind(src);
	if(file_size > max){
		fprintf(stderr, "Error. File too large for this media container. Aborting.\n");
		return(-1);		
	}

	fprintf(stderr, "Writing a file of size: %d bytes\n", file_size);

	bmp_data_set((uint8_t)(file_size & 0x000000FF), 1, media);
	bmp_data_set((uint8_t)((file_size & 0x0000FF00) >> 8), 2, media);
	bmp_data_set((uint8_t)((file_size & 0x00FF0000) >> 16), 3, media);
	bmp_data_set((uint8_t)((file_size & 0xFF000000) >> 24), 4, media);

	/* write file data */
	do{
		data_size = fread(datbuf, 1, FILE_WRITE_CHUNK_SIZE, src);
		for(i=0; i<FILE_WRITE_CHUNK_SIZE; i++){
			bmp_data_set(datbuf[i], index, media);
			index++;
			if(index > max){
				fprintf(stderr, "Error. File too large for media. Aborting.");
				return(-1);
			}
		}
	}while(data_size == FILE_WRITE_CHUNK_SIZE);
	
	return(0);
}




int bmp_read_file(FILE *media, FILE *out){
	
	uint64_t file_size = 0;
	uint64_t seg;
	file_size = (uint64_t)bmp_data_get(1, media);
	seg = (uint64_t)(bmp_data_get(2, media) << (8*1));
	file_size |= seg;
	seg = (uint64_t)(bmp_data_get(3, media) << (8*2));
	file_size |= seg;
	seg = (uint64_t)(bmp_data_get(4, media) << (8*3));
	file_size |= seg;
	fprintf(stderr, "Extracting file of size: %ld bytes.\n", file_size);

	int i=5; /* start indexing after tag and filesize bytes */
	uint8_t b, c=0;
	do{
		c = bmp_data_get(i, media);
		fprintf(out, "%c", c);
		i++;
	}while(--file_size > 0);

	return(0);
}


int bmp_write_stream(FILE *media, FILE *stream){
		
	/* add the tag for a string */
	bmp_data_set(STR_TAG, 0, media);	

	int i=1; /* start at one to skip tag */
	uint8_t b;
	char c;
	do{
		c = fgetc(stream);
		bmp_data_set(c, i, media);
		i++;
	}while(!feof(stream));
	bmp_data_set(0, i, media);
	
	return(0);
}



int bmp_read(FILE *media, FILE *out){
	int data_start = get_bmp_data_start(media);
	fseek(media, data_start, SEEK_SET);

	uint8_t tag;

	/* check tag */
	tag = bmp_data_get(0, media);
	if(tag != STR_TAG && tag != BIN_TAG){
		fprintf(stderr, "Error: Media file was never modified. Tag: [%02X]\n", tag);
		return(-1);
	}

	if(tag == BIN_TAG){
		bmp_read_file(media, out);
		return(0);
	}
	
	if(tag == STR_TAG){
		bmp_read_string(media, out);
		return(0);
	}
	
	fprintf(stderr, "Error: Issue with source code in bmp_read().\n");
	return(-1);
}


int bmp_write(FILE *media, FILE *in, char *str, int stream){
	int data_start = get_bmp_data_start(media);
	fseek(media, data_start, SEEK_SET);
	
	if(str != NULL){
		bmp_write_string(media, str);
		return(0);
	}
	else if(in == stdin || stream){
		if(in == NULL){
			fprintf(stderr, "Error: input stream is null.\n");
		}
		bmp_write_stream(media, in);
		return(0);
	}
	else if(in != NULL){
		bmp_write_file(media, in);
		return(0);
	}

	fprintf(stderr, "Error: Invalid input to bmp_write. No input was given.\n");
	return(-1);
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





