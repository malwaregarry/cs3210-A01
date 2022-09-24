#include <pthread.h>
#include <omp.h>
#include <algorithm>
#include <queue>
#include <map>
#include <unordered_map>
#include "util.h"
#include "DataObjects_bonus.h"
#define FORWARDS 0
#define BACKWARDS 1

using std::map;
using std::min;
using std::unordered_map;

vector<Station> stations;
unordered_map<int, Edge> edges;
vector<map<string, Train *>> train_collection(3);
vector<string> line_names[3];
int num_line_stations[3];
unordered_map<string, int> name_to_id;

void initialize_stations(size_t num_stations)
{
  size_t row_size = num_stations - 1;
  for (int color = 0; color < 3; color++)
  {
    for (int i = 0; i < num_line_stations[color]; i++)
    {
      int curr_id = name_to_id.at(line_names[color][i]);
      int forward_id = i < num_line_stations[color] - 1
                           ? name_to_id.at(line_names[color][i + 1])
                           : -1;
      int backward_id = i > 0
                            ? name_to_id.at(line_names[color][i - 1])
                            : -1;
      Station *station = &stations[curr_id];
      if (forward_id != -1)
      {
        station->edges[color * 2] = make_tuple(&stations[forward_id], &edges.at(curr_id * row_size + forward_id));
      }
      else
      {
        station->edges[color * 2] = make_tuple(nullptr, nullptr);
      }
      if (backward_id != -1)
      {
        station->edges[color * 2 + 1] = make_tuple(&stations[backward_id], &edges.at(curr_id * row_size + backward_id));
      }
      else
      {
        station->edges[color * 2 + 1] = make_tuple(nullptr, nullptr);
      }
    }
  }
}

void create_stations(int num_stations, const vector<string> &station_names, const std::vector<size_t> &popularities, const adjacency_matrix &mat)
{
  stations.reserve(num_stations);
  size_t row_size = num_stations - 1;
  for (int i = 0; i < num_stations; ++i)
  {
    Station temp(i, station_names[i], popularities[i]);
    stations.emplace_back(temp);
    name_to_id.insert({station_names[i], i});
  }
  for (int src_id = 0; src_id < num_stations; src_id++)
  {
    for (int des_id = 0; des_id < num_stations; des_id++)
    {
      if (mat[src_id][des_id] > 0)
      {
        string track_name = station_names[src_id] + "->" + station_names[des_id];
        edges.insert({src_id * row_size + des_id, Edge(Platform(), TrainTrack(track_name, mat[src_id][des_id]))});
      }
    }
  }
}

void print_state(size_t tick)
{
  string toPrint = to_string(tick) + ": ";

  for (const auto &i : train_collection[2])
  {
    toPrint += i.second->train_name + "-" + i.second->train_state + " ";
  }
  for (const auto &i : train_collection[0])
  {
    toPrint += i.second->train_name + "-" + i.second->train_state + " ";
  }
  for (const auto &i : train_collection[1])
  {
    toPrint += i.second->train_name + "-" + i.second->train_state + " ";
  }
  cout << toPrint << endl;
}

void spawn_trains_if_needed(int tick, int num_green_trains, int num_yellow_trains, int num_blue_trains, int num_line_stations[3], vector<Train> &train_threads)
{
  static int curr_train_id = 0;
  static int num_trains_created[3] = {0, 0, 0};
  string color_indicator[3] = {"g", "y", "b"};
  int num_trains_required[3] = {num_green_trains, num_yellow_trains, num_blue_trains};

  for (int color = 0; color < 3; color++)
  {
    int to_create = min((int)num_trains_required[color] - num_trains_created[color], 2);
    for (int dir = 0; dir < to_create; dir++)
    {
      size_t station_index = dir == FORWARDS ? 0 : num_line_stations[color] - 1;
      int station_id = name_to_id.at(line_names[color][station_index]);
      Station *station = &stations[station_id];
      string train_name = color_indicator[color] + to_string(curr_train_id);
      string train_state = station->station_name + "#";
      get<1>(station->edges[color * 2 + dir])->platform.holding_area.emplace(WaitingTrain{curr_train_id, color, tick}); // error here
      train_threads[curr_train_id] = Train(curr_train_id, color, train_name, train_state, dir);
      train_collection[color].insert({train_name, &train_threads[curr_train_id]});
      num_trains_created[color] += 1;
      curr_train_id += 1;
    }
  }
}

void simulate(int_fast32_t num_stations,
              const vector<string> &station_names,
              const std::vector<size_t> &popularities,
              const adjacency_matrix &mat,
              const vector<string> &green_station_names,
              const vector<string> &yellow_station_names,
              const vector<string> &blue_station_names, int ticks,
              int_fast32_t num_green_trains, int_fast32_t num_yellow_trains,
              int_fast32_t num_blue_trains, int_fast32_t num_lines)
{
  num_line_stations[0] = green_station_names.size();
  num_line_stations[1] = yellow_station_names.size();
  num_line_stations[2] = blue_station_names.size();
  line_names[0] = green_station_names;
  line_names[1] = yellow_station_names;
  line_names[2] = blue_station_names;
  create_stations(num_stations, station_names, popularities, mat);
  initialize_stations(num_stations);
  int stations_count_trains = num_green_trains + num_yellow_trains + num_blue_trains;
  vector<Train> train_threads(stations_count_trains);
  omp_set_num_threads(8);
  omp_set_schedule(omp_sched_static, 32); // chunksize

#pragma omp parallel
  for (int_fast32_t tick = 0; tick < ticks; tick++)
  {
#pragma omp single
    spawn_trains_if_needed(tick, num_green_trains, num_yellow_trains, num_blue_trains, num_line_stations, train_threads);
#pragma omp for
    for (int_fast32_t i = 0; i < num_stations; i++)
      stations[i].run_traintrack(tick);
#pragma omp for
    for (int_fast32_t i = 0; i < num_stations; i++)
      stations[i].run_platform();
#pragma omp for
    for (int_fast32_t i = 0; i < num_stations; i++)
      stations[i].run_holdingarea(train_threads);
#pragma omp single
    if (ticks - tick <= num_lines)
      print_state(tick);
  }
  stations.clear();
  edges.clear();
  train_threads.clear();
}

vector<string> extract_station_names(string &line)
{
  constexpr char space_delimiter = ' ';
  vector<string> stations{};
  line += ' ';
  size_t pos;
  while ((pos = line.find(space_delimiter)) != string::npos)
  {
    stations.push_back(line.substr(0, pos));
    line.erase(0, pos + 1);
  }
  return stations;
}

int main(int argc, char const *argv[])
{
  using std::cout;

  if (argc < 2)
  {
    std::cerr << argv[0] << " <input_file>\n";
    std::exit(1);
  }

  std::ifstream ifs(argv[1], std::ios_base::in);
  if (!ifs.is_open())
  {
    std::cerr << "Failed to open " << argv[1] << '\n';
    std::exit(2);
  }

  // Read S
  size_t S;
  ifs >> S;

  // Read station names.
  string station;
  std::vector<string> station_names{};
  station_names.reserve(S);
  for (size_t i = 0; i < S; ++i)
  {
    ifs >> station;
    station_names.emplace_back(station);
  }

  // Read P popularity
  size_t p;
  std::vector<size_t> popularities{};
  popularities.reserve(S);
  for (size_t i = 0; i < S; ++i)
  {
    ifs >> p;
    popularities.emplace_back(p);
  }

  // Form adjacency mat
  adjacency_matrix mat(S, std::vector<size_t>(S));
  for (size_t src{}; src < S; ++src)
  {
    for (size_t dst{}; dst < S; ++dst)
    {
      ifs >> mat[src][dst];
    }
  }

  ifs.ignore();

  string stations_buf;

  std::getline(ifs, stations_buf);
  auto green_station_names = extract_station_names(stations_buf);

  std::getline(ifs, stations_buf);
  auto yellow_station_names = extract_station_names(stations_buf);

  std::getline(ifs, stations_buf);
  auto blue_station_names = extract_station_names(stations_buf);

  // N time ticks
  size_t N;
  ifs >> N;

  // g,y,b number of trains per line
  size_t g, y, b;
  ifs >> g;
  ifs >> y;
  ifs >> b;

  size_t num_lines;
  ifs >> num_lines;

  simulate((int_fast32_t)S, station_names, popularities, mat, green_station_names,
           yellow_station_names, blue_station_names, (int_fast32_t)N, (int_fast32_t)g, (int_fast32_t)y, (int_fast32_t)b, (int_fast32_t)num_lines);

  return 0;
}
