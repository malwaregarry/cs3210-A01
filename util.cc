
#include "util.h"

void debug(size_t num_stations,
           const vector<string> &station_names,
           const std::vector<size_t> &popularities,
           const adjacency_matrix &mat,
           const vector<string> &green_station_names,
           const vector<string> &yellow_station_names,
           const vector<string> &blue_station_names, size_t ticks,
           size_t num_green_trains, size_t num_yellow_trains,
           size_t num_blue_trains, size_t num_lines)
{
    std::cout << num_stations << '\n';

    for (size_t i{}; i < num_stations; ++i)
    {
        std::cout << station_names[i] << ' ' << popularities[i] << ' ';
    }
    std::cout << '\n';

    for (size_t i{}; i < num_stations; ++i)
    {
        for (size_t j{}; j < num_stations; ++j)
        {
            std::cout << mat[i][j] << ' ';
        }
        std::cout << '\n';
    }

    for (const auto &stn : green_station_names)
    {
        std::cout << stn << ' ';
    }
    std::cout << '\n';

    for (const auto &stn : yellow_station_names)
    {
        std::cout << stn << ' ';
    }
    std::cout << '\n';

    for (const auto &stn : blue_station_names)
    {
        std::cout << stn << ' ';
    }
    std::cout << '\n';

    std::cout << ticks << '\n';
    std::cout << num_green_trains << '\n';
    std::cout << num_yellow_trains << '\n';
    std::cout << num_blue_trains << '\n';

    std::cout << num_lines << '\n';
}