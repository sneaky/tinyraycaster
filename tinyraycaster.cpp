#include <iostream>
#include <fstream>
#include <vector>
#include <cassert>
#include <cmath>
#include <cstdint>
#include <sstream>
#include <iomanip>

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
			if (cx >= img_w || cy >= img_h) continue;
			assert(cx < img_w && cy < img_h);
			img[cx + cy * img_w] = color;
		}
	}
}

int main() {
	const size_t win_w = 1024;
	const size_t win_h = 512;
	vector<uint32_t> framebuffer(win_w * win_h, pack_color(255, 255, 255));
	
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
	
	float player_x = 3.456;
	float player_y = 2.345;
	float player_a = 1.523;
	const float fov = M_PI/3;

	const size_t ncolors = 10;
	vector<uint32_t> colors(ncolors);
	for (size_t i = 0; i < ncolors; i++) {
		colors[i] = pack_color(rand()%255, rand()%255, rand()%255);
	}

	const size_t rect_w = win_w / (map_w * 2);
	const size_t rect_h = win_h / map_h;
	
	/*
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
        */

	for (size_t frame = 0; frame < 360; frame++) {
		stringstream ss;
		ss << setfill('0') << setw(5) << frame << ".ppm";
		player_a += 2 * M_PI / 360;

		framebuffer = vector<uint32_t>(win_w * win_h, pack_color(255, 255, 255)); // clear screen
		
		for (size_t j = 0; j < map_h; j++) {
			for (size_t i = 0; i < map_w; i++) {
				if (map[i + j * map_w] == ' ') continue;
				size_t rect_x = i * rect_w;
				size_t rect_y = j * rect_h;
				size_t icolor = map[i + j * map_w] - '0';
				assert(icolor < ncolors);
				draw_rect(framebuffer, win_w, win_h, rect_x, rect_y, rect_w, rect_h, colors[icolor]);
			}
		}
		for (size_t i = 0; i < win_w / 2; i++) {
			float angle = player_a - fov/2 + fov * i / static_cast<float>(win_w / 2);
			for (float t = 0; t < 20; t += .01) {
				float cx = player_x + t*cos(angle);
				float cy = player_y + t*sin(angle);
				//if (map[static_cast<int>(cx) + static_cast<int>(cy) * map_w] != ' ') break;

				size_t pix_x = cx * rect_w;
				size_t pix_y = cy * rect_h;
				framebuffer[pix_x + pix_y * win_w] = pack_color(160, 160, 160);

				if (map[static_cast<int>(cx) + static_cast<int>(cy) * map_w] != ' ') {
					size_t icolor = map[static_cast<int>(cx) + static_cast<int>(cy) * map_w] - '0';
					assert(icolor < ncolors);
					size_t column_height = win_h / t;
					draw_rect(framebuffer, win_w, win_h, win_w / 2 + i, win_h / 2 - column_height / 2, 1, column_height, pack_color(0, 255, 255));
					break;
				}
			}
		}
		drop_ppm_image(ss.str(), framebuffer, win_w, win_h);
	}

	//draw_rect(framebuffer, win_w, win_h, player_x * rect_w, player_y * rect_h, 5, 5, pack_color(255, 255, 255));
	
	
	//drop_ppm_image("./out.ppm", framebuffer, win_w, win_h);

	return 0;
}
