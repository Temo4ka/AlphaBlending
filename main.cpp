#include <SFML/Graphics.hpp>

#include "config.h"
#include "header.h"

int main()
{
    sf::RenderWindow window(sf::VideoMode(WINDOW_WIDTH, WINDOW_HEIGHT), "AlphaBlending");
	
	sf::Image image = {};
	image.create(WINDOW_WIDTH, WINDOW_HEIGHT, sf::Color::Green);

    BMP_File back  = {};
    ERR_EXE(readBMP(&back , BACK_PICTURE), logs);

    BMP_File front = {};
	ERR_EXE(readBMP(&front, FRONT_PICTURE), logs);

	if (front.pixels == nullptr) ERR_EXE(NullptrCaught, logs);
	if (back.pixels  == nullptr) ERR_EXE(NullptrCaught, logs);

	noormalizeAdress(&front.pixels);
	noormalizeAdress(&back.pixels);

    while (window.isOpen())
    {
        sf::Event event;
        while (window.pollEvent(event))
        {
            if (event.type == sf::Event::Closed)
                window.close();
        }
		
		for (int cur = 0; cur < PIXEL_NUM; cur ++) {
			//_8_i FR = {};
			//_8_i BK = {};

			//FR.data = _mm256_load_si256((const __m256*) front.pixels + cur * sizeof(int));
			//BK.data = _mm256_load_si256((const __m256*) back.pixels  + cur * sizeof(int));

			int y = WINDOW_HEIGHT - cur / WINDOW_WIDTH - 1;
			int x = cur % WINDOW_WIDTH;
			
			unsigned trans = front.pixels[cur].trans;
			unsigned  curR = (trans * front.pixels[cur].red   + (255 - trans) * back.pixels[cur].red  ) >> 8;
			unsigned  curG = (trans * front.pixels[cur].green + (255 - trans) * back.pixels[cur].green) >> 8;
			unsigned  curB = (trans * front.pixels[cur].blue  + (255 - trans) * back.pixels[cur].blue ) >> 8;

			if (curB > 255)
				curB = curB;
				
			image.setPixel(x, y, sf::Color(curR, curG, curB));
		}	

        drawImage(&window, image);
    }

    return 0;
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

void noormalizeAdress(Pixel** addr) {
	while (int(*addr) % 16) (*addr)++;

	return;
}

/*
g++ -I"library path"\SFML-2.5.1\include -c main.cpp -lm -o main.o

g++ -L"library path"\SFML-2.5.1\lib .\main.o -o name_file.exe -lmingw32 -lsfml-graphics -lsfml-window -lsfml-system -lsfml-audio -lsfml-main -mwindows 
*/
