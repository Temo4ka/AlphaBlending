#pragma once
#include <immintrin.h>

enum Errors {
	     Ok       =     0   ,
	NullptrCaught = (1 << 0),
	 FopenError   = (1 << 1),
	 FreadError   = (1 << 2),
};

struct Pixel {
	unsigned char blue  = 0;
	unsigned char green = 0;
	unsigned char  red  = 0;
	unsigned char trans = 0;
};

union _8_i {
	   __m256i      data;
	    short      sdata[16];
	     int       idata[8]  = { 0, 0, 0, 0, 0, 0, 0, 0 };
	 unsigned char cdata[32];
};

struct BMP_File {
	char*  header = (char*)  calloc(HEADER_SIZE, sizeof(char ));

	Pixel* pixels = (Pixel*) calloc( PIXEL_NUM + 32 , sizeof(Pixel));
};

FILE* logs = fopen("logs.txt", "w");

#define ERR_EXE(ERROR, STREAM) {  						  \
	if (ERROR) {								           \
		if (STREAM != nullptr)						        \
			fprintf(STREAM, "Error %d Occured!\n", ERROR);   \
		return 0;											  \
	}														   \
}       												        

int readBMP(BMP_File* pic, const char* filename);

void drawImage(sf::RenderWindow* window, sf::Image image);

void normalizeAdress(Pixel** addr);

void countColors(int cur, sf::Image* image, BMP_File* front, BMP_File* back);

void colorPixels(int x, int y, sf::Image* image, const __m256i MASK_TRANS, const __m256i MASK_SHUFF, _8_i FR, _8_i BK);
