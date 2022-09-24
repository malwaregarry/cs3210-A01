#include <fstream>
#include <iostream>
#include <string>
#include <vector>

using std::string;
using std::vector;

using adjacency_matrix = std::vector<std::vector<size_t>>;

void debug(size_t num_stations,
           const vector<string> &station_names,
           const std::vector<size_t> &popularities,
           const adjacency_matrix &mat,
           const vector<string> &green_station_names,
           const vector<string> &yellow_station_names,
           const vector<string> &blue_station_names, size_t ticks,
           size_t num_green_trains, size_t num_yellow_trains,
           size_t num_blue_trains, size_t num_lines);