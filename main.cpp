#include <SFML/Graphics.hpp>

#include "config.h"
#include "header.h"

const __m256i MASK_TRANS_1 = _mm256_set_epi8( 255, 14, 255, 14, 255, 14, 255, 14, 255,  6, 255,  6, 255,  6, 255,  6,
											  255, 14, 255, 14, 255, 14, 255, 14, 255,  6, 255,  6, 255,  6, 255,  6 );

const __m256i MASK_TRANS_2 = _mm256_set_epi8( 255, 19, 255, 19, 255, 19, 255, 19, 255, 23, 255, 23, 255, 23, 255, 23,
											  255, 27, 255, 27, 255, 27, 255, 27, 255, 31, 255, 31, 255, 31, 255, 31 );

const __m256i MASK_SHUFF_1 = _mm256_set_epi8( 255, 0, 255, 1, 255,  2, 255,  3, 255,  4, 255,  5, 255,  6, 255,  7,
											  255, 8, 255, 9, 255, 10, 255, 11, 255, 12, 255, 13, 255, 14, 255, 15 );

const __m256i MASK_SHUFF_2 = _mm256_set_epi8( 255, 16, 255, 17, 255, 18, 255, 19, 255, 20, 255, 21, 255, 22, 255, 23,
											  255, 24, 255, 25, 255, 26, 255, 27, 255, 28, 255, 29, 255, 30, 255, 31 );

const __m256i     _255     = _mm256_set_epi16(255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255);

int main()
{
    sf::RenderWindow window(sf::VideoMode(WINDOW_WIDTH, WINDOW_HEIGHT), "AlphaBlending");
	
	sf::Image image = {};
	image.create(WINDOW_WIDTH, WINDOW_HEIGHT, sf::Color::Green);

    BMP_File back  = {};
	normalizeAdress(&back.pixels);
    ERR_EXE(readBMP(&back , BACK_PICTURE), logs);

    BMP_File front = {};
	normalizeAdress(&front.pixels);
	ERR_EXE(readBMP(&front, FRONT_PICTURE), logs);

	if (front.pixels == nullptr) ERR_EXE(NullptrCaught, logs);
	if (back.pixels  == nullptr) ERR_EXE(NullptrCaught, logs);

	for (int cur = 0; cur < PIXEL_NUM; cur += 8) {
		countColors(cur, &image, &front, &back);
		front.pixels += 8;
		back.pixels += 8;
	}

    while (window.isOpen())
    {
        sf::Event event;
        while (window.pollEvent(event))
        {
            if (event.type == sf::Event::Closed)
                window.close();
        }

        drawImage(&window, image);
    }

    return 0;
}

void countColors(const int cur, sf::Image *image, BMP_File *front, BMP_File *back) {
	_8_i  FR   = {};
	_8_i  BK   = {};
	_8_i FR_l  = {};
	_8_i BK_l  = {};
	_8_i FR_h  = {};
	_8_i BK_h  = {};
	_8_i Col_l = {};
	_8_i Col_h = {};
	_8_i curCl = {};

	_8_i trans_l = {};
	_8_i trans_h = {};

	int x =         cur % WINDOW_WIDTH            ;
	int y = WINDOW_HEIGHT - cur / WINDOW_WIDTH - 1;

	FR.data = _mm256_loadu_si256((const __m256i*) front->pixels);
	BK.data = _mm256_loadu_si256((const __m256i*)  back->pixels);

	//separate high and low bytes
	FR_l.data = _mm256_cvtepu8_epi16(_mm256_extracti128_si256(FR.data, 1));
	FR_h.data = _mm256_cvtepu8_epi16(_mm256_extracti128_si256(FR.data, 0));

	BK_l.data = _mm256_cvtepu8_epi16(_mm256_extracti128_si256(BK.data, 1));
	BK_h.data = _mm256_cvtepu8_epi16(_mm256_extracti128_si256(BK.data, 0));

	trans_l.data = _mm256_shuffle_epi8(FR_l.data, MASK_TRANS_1);
	trans_h.data = _mm256_shuffle_epi8(FR_h.data, MASK_TRANS_1);

	//front * alpha
	FR_l.data = _mm256_mullo_epi16(FR_l.data, trans_l.data);
	FR_h.data = _mm256_mullo_epi16(FR_h.data, trans_h.data);

	//back * (255 - alpha)
	BK_l.data = _mm256_mullo_epi16(BK_l.data, _mm256_subs_epu16(_255, trans_l.data));
	BK_h.data = _mm256_mullo_epi16(BK_h.data, _mm256_subs_epu16(_255, trans_h.data));

	//back + front
	Col_l.data = _mm256_add_epi16(FR_l.data, BK_l.data);
	Col_h.data = _mm256_add_epi16(FR_h.data, BK_h.data);

	__m256i move_mask = _mm256_set_epi8(15, 13, 11, 9, 7, 5, 3, 1,
		255, 255, 255, 255, 255, 255, 255, 255,
		255, 255, 255, 255, 255, 255, 255, 255,
		15, 13, 11, 9, 7, 5, 3, 1);


	Col_l.data = _mm256_shuffle_epi8(Col_l.data, move_mask);
	Col_h.data = _mm256_shuffle_epi8(Col_h.data, move_mask);


	curCl.data = _mm256_set_m128i(_mm_add_epi8(_mm256_extracti128_si256(Col_l.data, 0), _mm256_extracti128_si256(Col_l.data, 1)),
								_mm_add_epi8(_mm256_extracti128_si256(Col_h.data, 0), _mm256_extracti128_si256(Col_h.data, 1)));

	for (int curPx = 0; curPx < 32; curPx += 4) {
		image->setPixel(x, y, sf::Color(curCl.cdata[curPx + 2], curCl.cdata[curPx + 1], curCl.cdata[curPx]));
		x++;
	}
}

int readBMP(BMP_File *pic, const char* filename) {
	if (filename == nullptr) return NullptrCaught;
	if (  pic    == nullptr) return NullptrCaught;

    FILE* stream = fopen(filename, "rb");
    if (stream == nullptr) return FopenError;

    fread(pic->header, sizeof(char ), HEADER_SIZE, stream);
    if (pic->header == nullptr) return FreadError;

    fread(pic->pixels, sizeof(Pixel),  PIXEL_NUM , stream);
    if (pic->pixels == nullptr) return FreadError;

    return Ok;
}

void drawImage(sf::RenderWindow* window, sf::Image image) {
	sf::Texture texture = {};
	texture.loadFromImage(image);
	sf::Sprite sprite = {};
	sprite.setTexture(texture);
	window->draw(sprite);
	window->display();
}

void normalizeAdress(Pixel** addr) {
	while (int(*addr) % 32) (*addr) = (Pixel *) (((char *) (*addr)) + 1);

	return;
}

/*
g++ -I"library path"\SFML-2.5.1\include -c main.cpp -lm -o main.o

g++ -L"library path"\SFML-2.5.1\lib .\main.o -o name_file.exe -lmingw32 -lsfml-graphics -lsfml-window -lsfml-system -lsfml-audio -lsfml-main -mwindows 
*/