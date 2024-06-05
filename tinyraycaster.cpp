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
#include "sprite.h"

int wall_x_texture_coord(const float hitx, const float hity, Texture &texture_walls) {
	float x = hitx - floor(hitx + 0.5); // x and y contain (signed) fractional parts of hitx and hity,
	float y = hity - floor(hity + 0.5); // they vary between -0.5 and +0.5, and one of them is supposed to be very close to 0
	int texture = x * texture_walls.size;
	
	if (std::abs(y) > std::abs(x)) { // determine whether we hit a vertical or horizontal wall
		texture = y * texture_walls.size;
	}
	
	if (texture < 0) { // handle case where x_texture_coord can be negative
		texture += texture_walls.size;
	}
	assert(texture >= 0 && texture < static_cast<int>(texture_walls.size));
	
	return texture;
}

/*
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
*/

void map_show_sprite(Sprite& sprite, FrameBuffer &fb, Map& map) {
	const size_t rect_w = fb.w / (map.w * 2);
	const size_t rect_h = fb.h / map.h;
	fb.draw_rect(sprite.x * rect_w - 3, sprite.y * rect_h - 3, 6, 6, pack_color(255, 0, 0));
}

void render(FrameBuffer& fb, Map& map, Player& player, std::vector<Sprite> &sprites, Texture& texture_walls, Texture& texture_monster) {
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
			//size_t column_height = fb.h / (t * cos(angle - player.a));
			float dist = t * cos(angle - player.a);
			size_t column_height = fb.h / dist;
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

	for (size_t i = 0; i < sprites.size(); i++) {
		map_show_sprite(sprites[i], fb, map);
	}
}

int main() {
	FrameBuffer fb{1024, 512, std::vector<uint32_t>(1024 * 512, pack_color(255, 255, 255))};
	Player player{3.456, 2.345, 1.523, M_PI/3.0};
	Map map;
	Texture texture_walls("./walltext.png");
	Texture texture_monsters("./monsters.png");
	if (!texture_walls.count || !texture_monsters.count) {
		std::cerr << "Failed to load wall textures" << std::endl;
		return -1;
	}

	std::vector<Sprite> sprites{ {1.834, 8.765, 0}, {5.323, 5.365, 1}, {4.123, 10.265, 1} };
	
	/*
	for (size_t frame = 0; frame < 360; frame++) {
		std::stringstream ss;
		ss << std::setfill('0') << std::setw(5) << frame << ".ppm";
		player.a += 2 * M_PI / 360;
		render(fb, map, player, texture_walls);
		drop_ppm_image(ss.str(), fb.img, fb.w, fb.h);
	}
	*/

	render(fb, map, player, sprites, texture_walls, texture_monsters);
	drop_ppm_image("./out.ppm", fb.img, fb.w, fb.h);
	return 0;
}
