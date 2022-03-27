#include "tools.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"


/*
* Load a one dimensional array of the map as two dimensional array.
*/
Map load_map(std::string filepath, int map_width, int map_height) {
	
	std::ifstream stream(filepath, std::ios::binary);

	if (!stream.is_open()) {
		std::cout << "\nCould not load the map. Empty vector returned!\n";
		return {};
	}

	char chr;
	std::vector<int8_t> row;
	Map map;

	std::string some;

	while (stream.get(chr)) {

		row.push_back(chr - '0');

		if (row.size() == map_width) {
			map.push_back(row);
			row.clear();
		}

	}

	stream.close();

	return map;
}


/*
* Build chunks to create partitions on the map.
*/
Chunks create_chunks(const Map& map, int chunk_row_size, int chunk_col_size) {

	int map_row_size = map.size();

	if (map_row_size == 0) {
		std::cout << "\nEmpty map passed as argument. Empty vector returned!\n";
		return {};
	}

	int map_col_size = map[0].size();

	int chunk_width = map_col_size / chunk_col_size;
	int chunk_height = map_row_size / chunk_row_size;

	std::vector<int8_t> row;
	Chunk chunk;
	Chunk_Row chunk_row;
	Chunks chunks;


	for (size_t i = 0; i < chunk_row_size; i++)
	{
		for (size_t j = 0; j < chunk_col_size; j++)
		{
			for (size_t r = 0; r < chunk_height; r++)
			{
				for (size_t c = 0; c < chunk_width; c++)
				{
					row.push_back(map[i * chunk_row_size + r][j * chunk_col_size + c]);
				}
				chunk.push_back(row);
				row.clear();
			}
			chunk_row.push_back(chunk);
			chunk.clear();
		}
		chunks.push_back(chunk_row);
		chunk_row.clear();
	}

	return chunks;
}

bool is_in_boundaries(int row, int col, int map_width, int map_height) {
	return (row >= 0 && row < map_height&& col >= 0 && col < map_width);
}

bool is_not_blocked(const Map& map, int row, int col) {
	if (map[row][col] == 0) return true;
	else return false;
}

bool is_reached_to_dest(int row, int col, const map_pos& destination) {
	if (row == destination.map_row && col == destination.map_col) return true;
	else return false;
}

double distance_to_dest(int row, int col, const map_pos& destination) {
	int dx = row - destination.map_row;
	int dy = col - destination.map_col;
	double dist = std::sqrt(dx * dx + dy * dy);
	return dist;
}

Path set_path(map_pos_info** pos_info, Grid& start_point, Grid& destination) {

	int col = destination.map_coords.map_col;
	int row = destination.map_coords.map_row;

	Path path;

	int temp_col, temp_row;

	Grid dummy;

	while (!(pos_info[row][col].parent_col == col && pos_info[row][col].parent_row == row)) {
		dummy.map_coords.map_col = col;
		dummy.map_coords.map_row = row;
		dummy.map_pos_to_grid_pos(start_point.in_grid_col, start_point.in_grid_row);
		path.push_back(dummy);
		temp_col = pos_info[row][col].parent_col;
		temp_row = pos_info[row][col].parent_row;
		col = temp_col;
		row = temp_row;
	}

	path.push_back(start_point);

	std::reverse(path.begin(), path.end());

	return path;
}

Path find_any_angle_shorthest_path(const Map& map, Grid& start_point, Grid& destination) {

	int map_height = map.size();

	if (map_height == 0) {
		std::cout << "\nEmpty map passed as argument. Empty vector returned!\n";
		return {};
	}

	int map_width = map[0].size();

	start_point.calculate_real_map_pos();
	destination.calculate_real_map_pos();

	if (!is_in_boundaries(start_point.map_coords.map_row, start_point.map_coords.map_col, map_width, map_height)) {
		std::cout << "\nInvalid coords! Given \"Start Point\" coords are outside of map boundaries! Empty vector returned!\n";
		return {};
	}

	if (!is_in_boundaries(destination.map_coords.map_row, destination.map_coords.map_col, map_width, map_height)) {
		std::cout << "\nInvalid coords! Given \"Destination Point\" coords are outside of map boundaries! Empty vector returned!\n";
		return {};
	}

	if (!is_not_blocked(map, start_point.map_coords.map_row, start_point.map_coords.map_col)) {
		std::cout << "\nInvalid coords! Given \"Start Point\" coords are blocked! Empty vector returned!\n";
		return {};
	}

	if (!is_not_blocked(map, destination.map_coords.map_row, destination.map_coords.map_col)) {
		std::cout << "\nInvalid coords! Given \"Destination Point\" coords are blocked! Empty vector returned!\n";
		return {};
	}

	if (is_reached_to_dest(start_point.map_coords.map_row, start_point.map_coords.map_col, destination.map_coords)) {
		std::cout << "\nGiven \"Start Point\" coords are equal to destination point coords! Empty vector returned!\n";
		return {};
	}

	bool** closedList = new bool* [map_height];

	int row, col;

	for (row = 0; row < map_height; row++)
	{
		closedList[row] = new bool[map_width];
		for (col = 0; col < map_width; col++)
		{
			closedList[row][col] = false;
		}
	}

	map_pos_info** pos_info = new map_pos_info * [map_height];

	for (row = 0; row < map_height; row++)
	{
		pos_info[row] = new map_pos_info[map_width];
		for (col = 0; col < map_width; col++)
		{
			pos_info[row][col].f = FLT_MAX;
			pos_info[row][col].g = FLT_MAX;
			pos_info[row][col].h = FLT_MAX;
			pos_info[row][col].parent_col = -1;
			pos_info[row][col].parent_row = -1;
		}
	}

	col = start_point.map_coords.map_col;
	row = start_point.map_coords.map_row;
	pos_info[row][col].f = 0.0;
	pos_info[row][col].g = 0.0;
	pos_info[row][col].h = 0.0;
	pos_info[row][col].parent_col = col;
	pos_info[row][col].parent_row = row;


	std::set<map_pos_with_cost> openList;

	openList.insert({ 0.0, row, col });

	bool is_destination_found = false;

	double fNew, gNew, hNew;

	Path path;

	while (!openList.empty()) {
		map_pos_with_cost pos = *openList.begin();

		openList.erase(openList.begin());

		col = pos.map_col;
		row = pos.map_row;
		closedList[row][col] = true;

		//1
		if (is_in_boundaries(row - 1, col, map_width, map_height)) {

			if (is_reached_to_dest(row - 1, col, destination.map_coords)) {
				pos_info[row - 1][col].parent_col = col;
				pos_info[row - 1][col].parent_row = row;
				std::cout << "\nDestination is found\n";
				path = set_path(pos_info, start_point, destination);
				is_destination_found = true;

				for (row = 0; row < map_height; row++)
				{
					delete[] closedList[row];
				}

				delete[] closedList;

				for (row = 0; row < map_height; row++)
				{
					delete[] pos_info[row];
				}

				delete[] pos_info;

				return path;
			}
			else if ((closedList[row - 1][col] == false && is_not_blocked(map, row - 1, col)) == true) {
				gNew = pos_info[row][col].g + 1.0;
				hNew = distance_to_dest(row - 1, col, destination.map_coords);
				fNew = gNew + hNew;

				if (pos_info[row - 1][col].f == FLT_MAX || pos_info[row - 1][col].f > fNew) {
					openList.insert({ fNew, row - 1, col });

					pos_info[row - 1][col].f = fNew;
					pos_info[row - 1][col].g = gNew;
					pos_info[row - 1][col].h = hNew;
					pos_info[row - 1][col].parent_col = col;
					pos_info[row - 1][col].parent_row = row;
				}
			}
		}
		//2
		if (is_in_boundaries(row + 1, col, map_width, map_height)) {

			if (is_reached_to_dest(row + 1, col, destination.map_coords)) {
				pos_info[row + 1][col].parent_col = col;
				pos_info[row + 1][col].parent_row = row;
				std::cout << "\nDestination is found\n";
				path = set_path(pos_info, start_point, destination);
				is_destination_found = true;

				for (row = 0; row < map_height; row++)
				{
					delete[] closedList[row];
				}

				delete[] closedList;

				for (row = 0; row < map_height; row++)
				{
					delete[] pos_info[row];
				}

				delete[] pos_info;

				return path;
			}
			else if ((closedList[row + 1][col] == false && is_not_blocked(map, row + 1, col)) == true) {
				gNew = pos_info[row][col].g + 1.0;
				hNew = distance_to_dest(row + 1, col, destination.map_coords);
				fNew = gNew + hNew;

				if (pos_info[row + 1][col].f == FLT_MAX || pos_info[row + 1][col].f > fNew) {
					openList.insert({ fNew, row + 1, col });

					pos_info[row + 1][col].f = fNew;
					pos_info[row + 1][col].g = gNew;
					pos_info[row + 1][col].h = hNew;
					pos_info[row + 1][col].parent_col = col;
					pos_info[row + 1][col].parent_row = row;
				}
			}
		}
		//3
		if (is_in_boundaries(row, col + 1, map_width, map_height)) {

			if (is_reached_to_dest(row, col + 1, destination.map_coords)) {
				pos_info[row][col + 1].parent_col = col;
				pos_info[row][col + 1].parent_row = row;
				std::cout << "\nDestination is found\n";
				path = set_path(pos_info, start_point, destination);
				is_destination_found = true;

				for (row = 0; row < map_height; row++)
				{
					delete[] closedList[row];
				}

				delete[] closedList;

				for (row = 0; row < map_height; row++)
				{
					delete[] pos_info[row];
				}

				delete[] pos_info;

				return path;
			}
			else if ((closedList[row][col + 1] == false && is_not_blocked(map, row, col + 1)) == true) {
				gNew = pos_info[row][col].g + 1.0;
				hNew = distance_to_dest(row, col + 1, destination.map_coords);
				fNew = gNew + hNew;

				if (pos_info[row][col + 1].f == FLT_MAX || pos_info[row][col + 1].f > fNew) {
					openList.insert({ fNew, row, col + 1 });

					pos_info[row][col + 1].f = fNew;
					pos_info[row][col + 1].g = gNew;
					pos_info[row][col + 1].h = hNew;
					pos_info[row][col + 1].parent_col = col;
					pos_info[row][col + 1].parent_row = row;
				}
			}
		}
		//4
		if (is_in_boundaries(row, col - 1, map_width, map_height)) {

			if (is_reached_to_dest(row, col - 1, destination.map_coords)) {
				pos_info[row][col - 1].parent_col = col;
				pos_info[row][col - 1].parent_row = row;
				std::cout << "\nDestination is found\n";
				path = set_path(pos_info, start_point, destination);
				is_destination_found = true;

				for (row = 0; row < map_height; row++)
				{
					delete[] closedList[row];
				}

				delete[] closedList;

				for (row = 0; row < map_height; row++)
				{
					delete[] pos_info[row];
				}

				delete[] pos_info;

				return path;
			}
			else if ((closedList[row][col - 1] == false && is_not_blocked(map, row, col - 1)) == true) {
				gNew = pos_info[row][col].g + 1.0;
				hNew = distance_to_dest(row, col - 1, destination.map_coords);
				fNew = gNew + hNew;

				if (pos_info[row][col - 1].f == FLT_MAX || pos_info[row][col - 1].f > fNew) {
					openList.insert({ fNew, row, col - 1 });

					pos_info[row][col - 1].f = fNew;
					pos_info[row][col - 1].g = gNew;
					pos_info[row][col - 1].h = hNew;
					pos_info[row][col - 1].parent_col = col;
					pos_info[row][col - 1].parent_row = row;
				}
			}
		}
		//5
		if (is_in_boundaries(row - 1, col + 1, map_width, map_height)) {

			if (is_reached_to_dest(row - 1, col + 1, destination.map_coords)) {
				pos_info[row - 1][col + 1].parent_col = col;
				pos_info[row - 1][col + 1].parent_row = row;
				std::cout << "\nDestination is found\n";
				path = set_path(pos_info, start_point, destination);
				is_destination_found = true;

				for (row = 0; row < map_height; row++)
				{
					delete[] closedList[row];
				}

				delete[] closedList;

				for (row = 0; row < map_height; row++)
				{
					delete[] pos_info[row];
				}

				delete[] pos_info;

				return path;
			}
			else if ((closedList[row - 1][col + 1] == false && is_not_blocked(map, row - 1, col + 1)) == true) {
				gNew = pos_info[row][col].g + 1.414;
				hNew = distance_to_dest(row - 1, col + 1, destination.map_coords);
				fNew = gNew + hNew;

				if (pos_info[row - 1][col + 1].f == FLT_MAX || pos_info[row - 1][col + 1].f > fNew) {
					openList.insert({ fNew, row - 1, col + 1 });

					pos_info[row - 1][col + 1].f = fNew;
					pos_info[row - 1][col + 1].g = gNew;
					pos_info[row - 1][col + 1].h = hNew;
					pos_info[row - 1][col + 1].parent_col = col;
					pos_info[row - 1][col + 1].parent_row = row;
				}
			}
		}
		//6
		if (is_in_boundaries(row - 1, col - 1, map_width, map_height)) {

			if (is_reached_to_dest(row - 1, col - 1, destination.map_coords)) {
				pos_info[row - 1][col - 1].parent_col = col;
				pos_info[row - 1][col - 1].parent_row = row;
				std::cout << "\nDestination is found\n";
				path = set_path(pos_info, start_point, destination);
				is_destination_found = true;

				for (row = 0; row < map_height; row++)
				{
					delete[] closedList[row];
				}

				delete[] closedList;

				for (row = 0; row < map_height; row++)
				{
					delete[] pos_info[row];
				}

				delete[] pos_info;

				return path;
			}
			else if ((closedList[row - 1][col - 1] == false && is_not_blocked(map, row - 1, col - 1)) == true) {
				gNew = pos_info[row][col].g + 1.414;
				hNew = distance_to_dest(row - 1, col - 1, destination.map_coords);
				fNew = gNew + hNew;

				if (pos_info[row - 1][col - 1].f == FLT_MAX || pos_info[row - 1][col - 1].f > fNew) {
					openList.insert({ fNew, row - 1, col - 1 });

					pos_info[row - 1][col - 1].f = fNew;
					pos_info[row - 1][col - 1].g = gNew;
					pos_info[row - 1][col - 1].h = hNew;
					pos_info[row - 1][col - 1].parent_col = col;
					pos_info[row - 1][col - 1].parent_row = row;
				}
			}
		}
		//7
		if (is_in_boundaries(row + 1, col + 1, map_width, map_height)) {

			if (is_reached_to_dest(row + 1, col + 1, destination.map_coords)) {
				pos_info[row + 1][col + 1].parent_col = col;
				pos_info[row + 1][col + 1].parent_row = row;
				std::cout << "\nDestination is found\n";
				path = set_path(pos_info, start_point, destination);
				is_destination_found = true;

				for (row = 0; row < map_height; row++)
				{
					delete[] closedList[row];
				}

				delete[] closedList;

				for (row = 0; row < map_height; row++)
				{
					delete[] pos_info[row];
				}

				delete[] pos_info;

				return path;
			}
			else if ((closedList[row + 1][col + 1] == false && is_not_blocked(map, row + 1, col + 1)) == true) {
				gNew = pos_info[row][col].g + 1.414;
				hNew = distance_to_dest(row + 1, col + 1, destination.map_coords);
				fNew = gNew + hNew;

				if (pos_info[row + 1][col + 1].f == FLT_MAX || pos_info[row + 1][col + 1].f > fNew) {
					openList.insert({ fNew, row + 1, col + 1 });

					pos_info[row + 1][col + 1].f = fNew;
					pos_info[row + 1][col + 1].g = gNew;
					pos_info[row + 1][col + 1].h = hNew;
					pos_info[row + 1][col + 1].parent_col = col;
					pos_info[row + 1][col + 1].parent_row = row;
				}
			}
		}
		//8
		if (is_in_boundaries(row + 1, col - 1, map_width, map_height)) {

			if (is_reached_to_dest(row + 1, col - 1, destination.map_coords)) {
				pos_info[row + 1][col - 1].parent_col = col;
				pos_info[row + 1][col - 1].parent_row = row;
				std::cout << "\nDestination is found\n";
				path = set_path(pos_info, start_point, destination);
				is_destination_found = true;

				for (row = 0; row < map_height; row++)
				{
					delete[] closedList[row];
				}

				delete[] closedList;

				for (row = 0; row < map_height; row++)
				{
					delete[] pos_info[row];
				}

				delete[] pos_info;

				return path;
			}
			else if ((closedList[row + 1][col - 1] == false && is_not_blocked(map, row + 1, col - 1)) == true) {
				gNew = pos_info[row][col].g + 1.414;
				hNew = distance_to_dest(row + 1, col - 1, destination.map_coords);
				fNew = gNew + hNew;

				if (pos_info[row + 1][col - 1].f == FLT_MAX || pos_info[row + 1][col - +1].f > fNew) {
					openList.insert({ fNew, row + 1, col - 1 });

					pos_info[row + 1][col - 1].f = fNew;
					pos_info[row + 1][col - 1].g = gNew;
					pos_info[row + 1][col - 1].h = hNew;
					pos_info[row + 1][col - 1].parent_col = col;
					pos_info[row + 1][col - 1].parent_row = row;
				}
			}
		}

	}

	if (is_destination_found == false) std::cout << "\nFailed to find the Destinationn\n";

	for (row = 0; row < map_height; row++)
	{
		delete[] closedList[row];
	}

	delete[] closedList;
	
	for (row = 0; row < map_height; row++)
	{
		delete[] pos_info[row];
	}

	delete[] pos_info;

	return path;
}

RGBMap to_RGB(const Map& map) {

	int map_row_size = map.size();

	if (map_row_size == 0) {
		std::cout << "\nEmpty map passed as argument. Empty vector returned!\n";
		return {};
	}

	int map_col_size = map[0].size();

	RGBMap RGBmap = {};
	std::vector<unsigned int> pixels = {};

	for (size_t i = 0; i < map_row_size; i++)
	{
		for (size_t j = 0; j < map_col_size; j++)
		{
			if (map[i][j] == 1) pixels.push_back(0);
			else pixels.push_back(0xffffff);
		}
		RGBmap.push_back(pixels);
		pixels.clear();
	}

	return RGBmap;
}

RGBMap highlight_path(const RGBMap& map, const Path& path, unsigned int color_hex) {

	RGBMap highlighted = map;
	int path_size = path.size();

	bool yLonger;
	int incrementVal, endVal, shortLen, longLen, swap, decInc, j;

	for (size_t coord = 1; coord <= path_size - 1; coord++)
	{
		yLonger = false;
		shortLen = path[coord].map_coords.map_col - path[coord - 1].map_coords.map_col;
		longLen = path[coord].map_coords.map_row - path[coord - 1].map_coords.map_row;

		if (std::abs(shortLen) > std::abs(longLen)) {
			swap = shortLen;
			shortLen = longLen;
			longLen = swap;
			yLonger = true;
		}

		endVal = longLen;

		if (longLen < 0) {
			incrementVal = -1;
			longLen = -longLen;
		}
		else incrementVal = 1;

		if (longLen == 0) decInc = 0;
		else decInc = (shortLen << 16) / longLen;

		j = 0;

		if (yLonger) {
			for (int i = 0; i != endVal; i += incrementVal)
			{
				highlighted[path[coord - 1].map_coords.map_row + (j >> 16)][path[coord - 1].map_coords.map_col + i] = color_hex;
				j += decInc;
			}
		}
		else {
			for (int i = 0; i != endVal; i += incrementVal)
			{
				highlighted[path[coord - 1].map_coords.map_row + i][path[coord - 1].map_coords.map_col + (j >> 16)] = color_hex;
				j += decInc;
			}
		}
	}

	return highlighted;
}

void write_image(const RGBMap& map, std::string name, int channel_num) {

	int img_height = map.size();

	if (img_height == 0) {
		std::cout << "\nEmpty RGBMap passed as argument.\n";
		return;
	}

	int img_width = map[0].size();

	uint8_t* rgb_image = new uint8_t[img_width * img_height * channel_num];

	uint8_t* iterator = rgb_image;

	int rgb, value;

	for (size_t i = 0; i < img_height; ++i) {
		for (size_t j = 0; j < img_width; ++j) {
			rgb = map[i][j];
			for (int c(channel_num - 1); c >= 0; --c) {
				value = (rgb >> (8 * c)) & 0xFF;
				*iterator = value;
				iterator++;
			}
		}
	}

	stbi_write_png(name.c_str(), img_width, img_height, channel_num, rgb_image, img_width * channel_num);
	stbi_image_free(rgb_image);
}