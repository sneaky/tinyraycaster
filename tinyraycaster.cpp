#include <iostream>
#include <fstream>
#include <vector>
#include <cassert>
#include <cmath>
#include <cstdint>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

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

bool load_texture(const string filename, vector<uint32_t> &texture, size_t &texture_size, size_t &texture_cnt) {
	int nchannels = -1, w, h;
	unsigned char* pixmap = stbi_load(filename.c_str(), &w, &h, &nchannels, 0);
	if (!pixmap) {
		cerr << "Error: can not load the textures" << endl;
		return false;
	}

	if (nchannels != 4) {
		cerr << "Error: the texture must be a 32 bit image" << endl;
		stbi_image_free(pixmap);
		return false;
	}

	texture_cnt = w/h;
	texture_size = w / texture_cnt;

	if (w != h * static_cast<int>(texture_cnt)) {
		cerr << "Error: the texture file must contain N square textures packed horizontally" << endl;
		stbi_image_free(pixmap);
		return false;
	}

	texture = vector<uint32_t>(w * h);
	for (int j = 0; j < h; j++) {
		for (int i = 0; i < w; i++) {
			uint8_t r = pixmap[(i + j * w) * 4];
			uint8_t g = pixmap[(i + j * w) * 4 + 1];
			uint8_t b = pixmap[(i + j * w) * 4 + 2];
			uint8_t a = pixmap[(i + j * w) * 4 + 3];
			texture[i + j * w] = pack_color(r, g, b, a);
		}
	}
	stbi_image_free(pixmap);
	return true;
}

vector<uint32_t> texture_column(const vector<uint32_t> &img, const size_t texsize, const size_t ntextures, const size_t texid, const size_t texcoord, const size_t column_height) {
	const size_t img_w = texsize * ntextures;
	const size_t img_h = texsize;
	assert(img.size() == img_w * img_h && texcoord < texsize && texid < ntextures);
	vector<uint32_t> column(column_height);
	for (size_t y = 0; y < column_height; y++) {
		size_t pix_x = texid * texsize + texcoord;
		size_t pix_y = (y * texsize) / column_height;
		column[y] = img[pix_x + pix_y * img_w];
	}

	return column;
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
			       "0   3   11100  0"\
			       "5   4   0      0"\
			       "5   4   1  00000"\
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

	vector<uint32_t> walltexture;
	size_t walltexture_size;
	size_t walltexture_cnt;
	if (!load_texture("./walltext.png", walltexture, walltexture_size, walltexture_cnt)) {
		cerr << "Failed to load wall textures" << endl;
		return -1;
	}

	const size_t rect_w = win_w / (map_w * 2);
	const size_t rect_h = win_h / map_h;
	
	for (size_t j = 0; j < map_h; j++) {
		for(size_t i = 0; i < map_w; i++) {
			if (map[i + j * map_w] == ' ') continue;
			size_t rect_x = i * rect_w;
			size_t rect_y = j * rect_h;
			size_t texid = map[i + j * map_w] - '0';
			assert(texid < walltexture_cnt);
			draw_rect(framebuffer, win_w, win_h, rect_x, rect_y, rect_w, rect_h, walltexture[texid * walltexture_size]); // color taken from the upper left pixel of the texture #texid
		}
	}

	for (size_t i = 0; i < win_w / 2; i++) {
		float angle = player_a - fov / 2 + fov * i / static_cast<float>(win_w / 2);
		for (float t = 0; t < 20; t += 0.01) {
			float cx = player_x + t * cos(angle);
			float cy = player_y + t * sin(angle);


			size_t pix_x = cx * rect_w;
			size_t pix_y = cy * rect_h;
			framebuffer[pix_x + pix_y * win_w] = pack_color(160, 160, 160); // visibility cone
			
			if (map[static_cast<int>(cx) + static_cast<int>(cy) * map_w] != ' ') { // ray touches a wall, so draw the vertical column to create illusion of 3D
				size_t texid = map[static_cast<int>(cx) + static_cast<int>(cy) * map_w] - '0';
				assert(texid < walltexture_cnt);
				size_t column_height = win_h / (t * cos(angle - player_a));
				float hitx = cx - floor(cx + .5);
				float hity = cy - floor(cy + .5);
				int x_texcoord = hitx * walltexture_size;
				if (abs(hity) > abs(hitx)) {
					x_texcoord = hity * walltexture_size;
				}
				if (x_texcoord < 0) x_texcoord += walltexture_size;
				assert(x_texcoord >= 0 && x_texcoord < static_cast<int>(walltexture_size));

				vector<uint32_t> column = texture_column(walltexture, walltexture_size, walltexture_cnt, texid, x_texcoord, column_height);
				pix_x = win_w / 2 + i;
				for (size_t j = 0; j < column_height; j++) {
					pix_y = j + win_h / 2 - column_height / 2;
					if (pix_y < 0 || pix_y >= static_cast<int>(win_h)) continue;
					framebuffer[pix_x + pix_y * win_w] = column[j];
				}
				break;
			}
		}
	}
	
	/*
	const size_t texture_id = 4; // draw the 4th texture on the screen
	for (size_t i = 0; i < walltexture_size; i++) {
		for (size_t j = 0; j < walltexture_size; j++) {
			framebuffer[i + j * win_w] = walltexture[i + texture_id * walltexture_size + j * walltexture_size * walltexture_cnt];
		}
	}
	*/

	//draw_rect(framebuffer, win_w, win_h, player_x * rect_w, player_y * rect_h, 5, 5, pack_color(255, 255, 255));
	
	
	drop_ppm_image("./out.ppm", framebuffer, win_w, win_h);

	return 0;
}
