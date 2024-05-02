#include <iostream>
#include <fstream>
#include <vector>
#include <cstdint>
#include <cassert>

using namespace std;

uint32_t pack_color(const uint8_t r, const uint8_t g, const uint8_t b, const uint8_t a=255) {
	return (a << 24) + (b << 16) + (g << 8) + r;
}

void unpack_color(const uint32_t &color, uint8_t &r, uint8_t &g, uint8_t &b, uint8_t &a) {
	r = (color >> 0) & 255;
	g = (color >> 8) & 255;
	b = (color >> 16) & 255;
	a = (color >> 24) & 255;
}

void drop_ppm_image(const string filename, const vector<uint32_t> &image, const size_t w, const size_t h) {
	assert(image.size() == w * h);
	ofstream ofs(filename, ios::binary);
	ofs << "P6\n" << w << " " << h << "\n255\n";
	for (size_t i = 0; i < h * w; ++i) {
		uint8_t r, g, b, a;
		unpack_color(image[i], r, g, b, a);
		ofs << static_cast<char>(r) << static_cast<char>(g) << static_cast<char>(b);
	}
	ofs.close();
}

void draw_rect(vector<uint32_t> &img, const size_t img_w, const size_t img_h, const size_t x, const size_t y, const size_t w, const size_t h, const uint32_t color) {
	assert(img.size() == img_w * img_h);
	for (size_t i = 0; i < w; i++) {
		for (size_t j = 0; j < h; j++) {
			size_t cx = x + i;
			size_t cy = y + j;
			assert(cx < img_w && cy < img_h);
			img[cx + cy * img_w] = color;
		}
	}
}

int main() {
	const size_t win_w = 512;
	const size_t win_h = 512;
	vector<uint32_t> framebuffer(win_w * win_h, 255);
	
	const size_t map_w = 16;
	const size_t map_h = 16;
	const char map[] =      "0000222222220000"\
			       "1              0"\
			       "1      11111   0"\
			       "1     0        0"\
			       "0     0  1110000"\
			       "0     3        0"\
			       "0   10000      0"\
			       "0   0   11100  0"\
			       "0   0   0      0"\
			       "0   0   1  00000"\
			       "0       1      0"\
			       "2       1      0"\
			       "0       0      0"\
			       "0 0000000      0"\
			       "0              0"\
			       "0002222222200000";

	assert(sizeof(map) == map_w * map_h + 1);

	for (size_t j = 0; j < win_h; j++) {
		for (size_t i = 0; i < win_w; i++) {
			uint8_t r = 255 * j / float(win_h);
			uint8_t g = 255 * i / float(win_w);
			uint8_t b = 0;
			framebuffer[i + j * win_w] = pack_color(r, g, b);
		}
	}

	const size_t rect_w = win_w / map_w;
	const size_t rect_h = win_h / map_h;
	for (size_t j = 0; j < map_w; j++) {
		for (size_t i = 0; i < map_w; i++) {
			if (map[i + j * map_w] == ' ') continue;
			size_t rect_x = i * rect_w;
			size_t rect_y = j * rect_h;

			switch(map[i + j * map_w]) {
				case '0':
					draw_rect(framebuffer, win_w, win_h, rect_x, rect_y, rect_w, rect_h, pack_color(0, 255, 255)); // light-blue
					break;
				case '1':
					draw_rect(framebuffer, win_w, win_h, rect_x, rect_y, rect_w, rect_h, pack_color(0, 0, 255)); // dark-blue
					break;
				case '2':
					draw_rect(framebuffer, win_w, win_h, rect_x, rect_y, rect_w, rect_h, pack_color(0, 128, 255)); // med-blue
					break;
				case '3':
					draw_rect(framebuffer, win_w, win_h, rect_x, rect_y, rect_w, rect_h, pack_color(255, 0, 255)); // pink
					break;
				default:
					continue;
			}
		}
	}

	drop_ppm_image("./out.ppm", framebuffer, win_w, win_h);

	return 0;
}
