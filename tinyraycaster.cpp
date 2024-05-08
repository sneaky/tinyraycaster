#define _USE_MATH_DEFINES
#include <cmath>
#include <iostream>
#include <vector>
#include <cstdint>
#include <cassert>
#include <sstream>
#include <iomanip>

#include "map.h"
#include "utils.h"
#include "player.h"
#include "framebuffer.h"
#include "textures.h"

int wall_x_texture_coord(const float x, const float y, Texture& texture_walls) {
	float hitx = x - floor(x + 0.05); // hitx and hity contain (signed) fractional parts of x and y,
	float hity = y - floor(y + 0.05); // they vary between -0.5 and +0.5, and one of them is supposed to be very close to 0
	int texture = hitx * texture_walls.size;
	
	if (std::abs(hity) > std::abs(hitx)) { // determine whether we hit a "vertical" or "horizontal" wall
		texture = hity * texture_walls.size;
	}

	if (texture < 0) { // handle case where x_texture_coord is negative
		texture += texture_walls.size;
	}
	assert(texture >= 0 && texture < static_cast<int>(texture_walls.size));

	return texture;
}

void render(FrameBuffer& fb, Map& map, Player& player, Texture& texture_walls) {
	fb.clear(pack_color(255, 255, 255)); // clear the screen
	
	const size_t rect_w = fb.w / (map.w * 2);
	const size_t rect_h = fb.h / map.h;
	for (size_t j = 0; j < map.h; j++) {
		for (size_t i = 0; i < map.w; i++) {
			if (map.is_empty(i, j)) continue;
			size_t rect_x = i * rect_w;
			size_t rect_y = j * rect_h;
			size_t texture_id = map.get(i, j);
			assert(texture_id < texture_walls.count);
			fb.draw_rect(rect_x, rect_y, rect_w, rect_h, texture_walls.get(0, 0, texture_id));
		}
	}

	for (size_t i = 0; i < fb.w / 2; i++) {
		float angle = player.a - player.fov / 2 + player.fov * i / static_cast<float>(fb.w / 2);
		for (float t = 0; t < 20; t += 0.01) {
			float x = player.x + t * cos(angle);
			float y = player.y + t * sin(angle);
			fb.set_pixel(x * rect_w, y * rect_h, pack_color(160, 160, 160));

			if (map.is_empty(x, y)) continue;

			size_t texture_id = map.get(x, y);
			assert(texture_id < texture_walls.count);
			size_t column_height = fb.h / (t * cos(angle - player.a));
			int x_texture_coord = wall_x_texture_coord(x, y, texture_walls);
			std::vector<uint32_t> column = texture_walls.get_scaled_column(texture_id, x_texture_coord, column_height);
			int pix_x = i + fb.w / 2;
			for (size_t j = 0; j < column_height; j++) {
				int pix_y = j + fb.h / 2 - column_height / 2;
				if (pix_y >= 0 && pix_y < static_cast<int>(fb.h)) {
					fb.set_pixel(pix_x, pix_y, column[j]);
				}
			}
			break;
		}
	}
}

int main() {
	FrameBuffer fb{1024, 512, std::vector<uint32_t>(1024 * 512, pack_color(255, 255, 255))};
	Player player{3.456, 2.345, 1.523, M_PI/3.0};
	Map map;
	Texture texture_walls("./walltext.png");
	if (!texture_walls.count) {
		std::cerr << "Failed to load wall textures" << std::endl;
		return -1;
	}

	for (size_t frame = 0; frame < 360; frame++) {
		std::stringstream ss;
		ss << std::setfill('0') << std::setw(5) << frame << ".ppm";
		player.a += 2 * M_PI / 360;
		render(fb, map, player, texture_walls);
		drop_ppm_image(ss.str(), fb.img, fb.w, fb.h);
	}

	return 0;
}
