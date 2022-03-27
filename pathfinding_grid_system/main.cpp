#include <iostream>
#include <chrono>

#include "tools.h"



int main() {

	/*load map*/
	auto t_start = std::chrono::high_resolution_clock::now();

	Map map(load_map("levenia_map_2.atr", 1024, 1024));
	/*or the other way*/
	//Map map = load_map("levenia_map_2.atr", 1024, 1024));

	auto t_now = std::chrono::high_resolution_clock::now();
	float time = std::chrono::duration_cast<std::chrono::milliseconds>(t_now - t_start).count();
	
	printf("\nLoading time of the map atr file from hdd to RAM: %f ms\n", time);
	/*load map*/

	/*find path*/
	Grid start_point1;
	start_point1.grid_col = 18000; //420 * 100
	start_point1.grid_row = 17000; //210 * 100
	start_point1.in_grid_col = 35; //x coord in the 100x100 matrix
	start_point1.in_grid_row = 22; //y coord in the 100x100 matrix
	/*or simply*/
	//start_point1 = { 42000, 21000, 35, 22 };
	start_point1.calculate_real_map_pos();

	Grid destination = { 77400, 86000, 10, 89 };
	destination.calculate_real_map_pos();

	t_start = std::chrono::high_resolution_clock::now();

	Path path = find_any_angle_shorthest_path(map, start_point1, destination); 

	t_now = std::chrono::high_resolution_clock::now();
	time = std::chrono::duration_cast<std::chrono::milliseconds>(t_now - t_start).count();

	printf("\nPath Finding time on the map: %f ms\n", time);
	/*find path*/


	/*highlight path*/
	t_start = std::chrono::high_resolution_clock::now();

	RGBMap RGBmap(to_RGB(map));
	RGBmap = highlight_path(RGBmap, path);

	t_now = std::chrono::high_resolution_clock::now();
	time = std::chrono::duration_cast<std::chrono::milliseconds>(t_now - t_start).count();

	printf("\nHighlighting time of the path on the map: %f ms\n", time);
	/*highlight path*/

	/*write to hdd*/
	t_start = std::chrono::high_resolution_clock::now();

	write_image(RGBmap, "shorhest_path.png");

	t_now = std::chrono::high_resolution_clock::now();
	time = std::chrono::duration_cast<std::chrono::milliseconds>(t_now - t_start).count();

	printf("\nWriting time of the map image from RAM to hdd: %f ms\n", time);
	/*write to hdd*/

	return 0;
}