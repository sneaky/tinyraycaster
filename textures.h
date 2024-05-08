#ifndef TEXTURES_H
#define TEXTURES_H

#include <cstdlib>
#include <vector>
#include <cstdint>
#include <string>

typedef struct Texture {
	size_t img_w, img_h;		// image dimensions
	size_t count, size;		// number of textures and size in pixels
	std::vector<uint32_t> img;	// textures storage
	
	Texture(const std::string filename);
	uint32_t& get(const size_t i, const size_t j, const size_t idx); // get pixel (i, j) from the texture idx
	std::vector<uint32_t> get_scaled_column(const size_t texture_id, const size_t texture_coord, const size_t column_height); // retrieve one column (texture_coord) from the texture_id and scale it to the destination size
} Texture;

#endif
