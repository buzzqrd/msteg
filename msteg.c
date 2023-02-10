#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#include <arpa/inet.h>

#define CHUNK_SIZE 32
#define FIELD_SIZE 4

#define STR_TAG 0x01
#define BIN_TAG 0x02



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




int print_bmp_data(FILE *fp){
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

int bmp_data_embed(char *str, FILE *fp){
	int data_start = get_bmp_data_start(fp);
	fseek(fp, data_start, SEEK_SET);
	
	if(strlen(str) >= get_max_data_size(fp)){
		fprintf(stderr, "This data is too large to store in this file.\n");
		return(-1);
	}

	/* add the tag for a string */
	bmp_data_set(STR_TAG, 0, fp);	

	int i=1;
	uint8_t b, c;
	char *s = str;
	while(*s != 0){
		bmp_data_set(*s, i, fp);
		i++;
		s++;
	}
	bmp_data_set(0, i, fp);

	return(0);
}


int bmp_data_extract_print(FILE *fp){
	int data_start = get_bmp_data_start(fp);
	fseek(fp, data_start, SEEK_SET);
	
	/* get the data type and make sure it is valid */
	uint8_t type = bmp_data_get(0, fp);
	if(type == BIN_TAG){
		fprintf(stderr, "This file contains binary data. Please dump it to a file.");
		return(-1);
	}
	else if(type != STR_TAG){
		fprintf(stderr, "This file was never modified.\n");
		return(-1);
	}			

	int i=1;
	uint8_t b, c=0;
	do{
		c = bmp_data_get(i, fp);
		fprintf(stdout, "%c", c);
		i++;
	}while(c != 0);
	printf("\n");

	return(0);
}


int bmp_file_embed(FILE *src, FILE *fp){
	int max = get_max_data_size(fp)-4;  
	uint8_t datbuf[16];
	int data_size = 0, i=0, index=5;

	/* embed the file tag */
	printf("Writing file data to image.\n");
	bmp_data_set(BIN_TAG, 0, fp);	


	/* embed the file size (4-bytes) */
	fseek(src, 0, SEEK_END);
	unsigned int file_size = ftell(src);
	rewind(src);
	if(file_size > max){
		fprintf(stderr, "Error. File too large. Aborting.");
		return(-1);		
	}

	printf("Writing file size: %d\n", file_size);

	bmp_data_set((uint8_t)(file_size & 0x000000FF), 1, fp);
	bmp_data_set((uint8_t)((file_size & 0x0000FF00) >> 8), 2, fp);
	bmp_data_set((uint8_t)((file_size & 0x00FF0000) >> 16), 3, fp);
	bmp_data_set((uint8_t)((file_size & 0xFF000000) >> 24), 4, fp);

	/* write file data */
	do{
		data_size = fread(datbuf, 1, 16, src);
		for(i=0; i<16; i++){
			bmp_data_set(datbuf[i], index, fp);
			index++;
			if(index > max){
				fprintf(stderr, "Error. File too large. Aborting.");
				return(-1);
			}
		}
	}while(data_size == 16);
	
	return(0);
}




int bmp_file_extract(FILE *fp){
	int data_start = get_bmp_data_start(fp);
	fseek(fp, data_start, SEEK_SET);
	
	/* get the data type and make sure it is valid */
	uint8_t type = bmp_data_get(0, fp);
	if(type == STR_TAG){
		fprintf(stderr, "This file contains text data.  Please read it with \"-r\".\n");
		return(-1);
	}
	else if(type != BIN_TAG){
		fprintf(stderr, "This file was never modified. Tag: %02x\n", type);
		return(-1);
	}			

	unsigned int file_size = 0;
	unsigned int seg;
	file_size = (unsigned int)bmp_data_get(1, fp);
	seg = (unsigned int)(bmp_data_get(2, fp) << (8*1));
	file_size |= seg;
	seg = (unsigned int)(bmp_data_get(3, fp) << (8*2));
	file_size |= seg;
	seg = (unsigned int)(bmp_data_get(4, fp) << (8*3));
	file_size |= seg;

	fprintf(stderr, "File size: %08x\n", file_size);

	int i=5; /* start indexing after tag and filesize bytes */
	uint8_t b, c=0;
	do{
		c = bmp_data_get(i, fp);
		fprintf(stdout, "%c", c);
		i++;
	}while(--file_size > 0);

	return(0);
}




int main(int argc, const char *argv[]){
	if(argc < 3){
		printf("Not enough args.\n");
		return(-1);
	}

	if(*argv[1] != '-'){
		printf("No valid operaiton flag detected.\n Please use \'-r/w/s.\'\n");
		return(-1);
	}


	FILE *fp = fopen(argv[2], "r+b"); /* open for reading and writing */

	uint8_t chunk[CHUNK_SIZE];
	memset(chunk, 0, CHUNK_SIZE);

	fread(chunk, 1, 8, fp);

	if(strcmpb(chunk, "BM", 2)){
		printf("This is not a BMP file.\n");
		fclose(fp);
		return(-1);
	}	

	if(!strcmp(argv[1], "-s")){
		print_bmp_data(fp);
	}
	else if(!strcmp(argv[1], "-r")){
		bmp_data_extract_print(fp);
	}
	else if(!strcmp(argv[1], "-w")){
		if(argc < 4){
			printf("Last argument needs to be a string to write.\n");
			fclose(fp);
			return(-1);
		}
		bmp_data_embed((char*)argv[3], fp);
		printf("Done embedding data.\n");
	}	
	else if(!strcmp(argv[1], "-f")){
		if(argc < 4){
			printf("Last argument needs to be a file to write.\n");
			fclose(fp);
			return(-1);
		}
		FILE *fsrc = fopen(argv[3], "rb");
		bmp_file_embed(fsrc, fp);
	}
	else if(!strcmp(argv[1], "-x")){
		bmp_file_extract(fp);
	}
	else{
		printf("Invalid option.\n");
		fclose(fp);	
		return(-1);
	}

	fclose(fp);	
	return(0);
}





