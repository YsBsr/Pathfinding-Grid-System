#pragma once

#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <math.h>
#include <set>

struct map_pos {
	int map_row; //y coord
	int map_col; //x coord
};

struct map_pos_with_cost {
	double f; //distance cost to destination
	int map_row; //y coord
	int map_col; //x coord

	bool operator<(const map_pos_with_cost& rhs) const noexcept {
		return this->f < rhs.f;
	}
};

struct map_pos_info {
	int parent_row; //y coord
	int parent_col; //x coord

	double f; //distance cost to destination --> f = g + h.
	double g; //exact cost of the path from the starting point to any vertex n.
	double h; //heuristic estimated cost from vertex n to the goal.
};

struct Grid {
	int grid_row; //y coord in the grid system
	int grid_col; //x coord in the grid system

	int in_grid_row = 0; //y coord in one grid, default 0
	int in_grid_col = 0; //x coord in one grid, default 0

	map_pos map_coords; // real x and y coords on the map;

	int grid_row_size = 100;
	int grid_col_size = 100; //100x100 matrix representation 

	void calculate_real_map_pos() {
		this->map_coords.map_col = this->grid_col / this->grid_col_size;
		this->map_coords.map_row = this->grid_row / this->grid_row_size;
	}

	void map_pos_to_grid_pos(int offset_col, int offset_row) {
		this->grid_col = this->map_coords.map_col * this->grid_col_size + offset_col;
		this->grid_row = this->map_coords.map_row * this->grid_row_size + offset_row;
		this->in_grid_col = offset_col;
		this->in_grid_row = offset_row;
	}

	void calculate_in_grid_pos() {
		if (this->in_grid_col == 0 && this->in_grid_row == 0) {
			this->in_grid_col = this->grid_col % this->grid_col_size;
			this->in_grid_row = this->grid_row % this->grid_row_size;
		}
		else {
			this->map_pos_to_grid_pos(this->in_grid_col, this->in_grid_row);
			this->in_grid_col = this->grid_col % this->grid_col_size;
			this->in_grid_row = this->grid_row % this->grid_row_size;
		}
	}

};

typedef std::vector<std::vector<int8_t>> Map;

typedef std::vector<std::vector<int8_t>> Chunk;
typedef std::vector<Chunk> Chunk_Row;
typedef std::vector<Chunk_Row> Chunks;

typedef std::vector<std::vector<unsigned int>> RGBMap;
typedef std::vector<Grid> Path;


Map load_map(std::string filepath, int map_width, int map_height);
Chunks create_chunks(const Map& map, int chunk_row_size, int chunk_col_size);

bool is_in_boundaries(int row, int col, int map_width, int map_height);
bool is_not_blocked(const Map& map, int row, int col);
bool is_reached_to_dest(int row, int col, const map_pos& destination);
double distance_to_dest(int row, int col, const map_pos& destination);
Path set_path(map_pos_info** pos_info, Grid& start_point, Grid& destination);
Path find_any_angle_shorthest_path(const Map& map, Grid& start_point, Grid& destination);

RGBMap to_RGB(const Map& map);
RGBMap highlight_path(const RGBMap& map, const Path& path, unsigned int color_hex = 0x0000ff); //you can put any hex color value to highligt the path, default blue

void write_image(const RGBMap& map, std::string name, int channel_num = 3);