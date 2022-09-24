using namespace std;

#include <string>
#include <vector>
#include <queue>
#include <omp.h>

#define FORWARDS 0
#define BACKWARDS 1

typedef struct Train
{
    int train_id;
    int color;
    string train_name;
    string train_state;
    int direction;

    Train()
    {
    }

    Train(int train_id, int color, string train_name, string train_state, int direction)
    {
        this->train_id = train_id;
        this->color = color;
        this->train_name = train_name;
        this->train_state = train_state;
        this->direction = direction;
    }
} Train;

typedef struct WaitingTrain
{
    int train_id;
    int color;
    int arrival_tick;
} WaitingTrain;

typedef struct TrainTrack
{
    string track_name;
    int distance;
    Train *curr_train;
    int count_down;

    TrainTrack() : curr_train(0)
    {
        this->count_down = 0;
    }

    TrainTrack(string track_name, int distance) : curr_train(0)
    {
        this->track_name = track_name;
        this->distance = distance;
        this->count_down = 0;
    }

    ~TrainTrack()
    {
        curr_train = NULL;
    }

} TrainTrack;

typedef struct Compare
{
    bool operator()(WaitingTrain a, WaitingTrain b)
    {
        if (a.arrival_tick == b.arrival_tick)
        {
            return a.train_id > b.train_id;
        }
        return a.arrival_tick > b.arrival_tick;
    };
} Compare;

typedef struct Platform
{
    priority_queue<WaitingTrain, vector<WaitingTrain>, Compare> holding_area;
    int count_down;
    bool is_ready;
    Train *curr_train;
    omp_lock_t mutex;

    Platform() : curr_train(0)
    {
        omp_init_lock(&mutex);
        this->count_down = 0;
        this->is_ready = false;
    }

    ~Platform()
    {
        curr_train = NULL;
        omp_destroy_lock(&mutex);
        holding_area = priority_queue<WaitingTrain, vector<WaitingTrain>, Compare>();
    }
} Platform;

typedef struct Station
{

    int station_id;
    string station_name;
    int popularity;
    Station *next[6];
    Platform *platforms[6];
    TrainTrack *train_tracks[6];

    Station(int station_id, string station_name, int popularity) : next(), platforms(), train_tracks()
    {
        this->station_id = station_id;
        this->station_name = station_name;
        this->popularity = popularity;
    }
    Station()
    {
    }
    ~Station()
    {
        for (int i = 0; i < 6; i++)
        {
            next[i] = NULL;
            platforms[i] = NULL;
            train_tracks[i] = NULL;
        }
    }

    void run_traintrack(int tick)
    {
        for (int color = 0; color < 3; color++)
        {
            for (int dir = 0; dir < 2; dir++)
            {
                TrainTrack *train_track = train_tracks[color * 2 + dir];
                if (train_track == NULL || train_track->curr_train == NULL || train_track->curr_train->color != color)
                    continue;
                if (train_track->count_down > 0)
                {
                    train_track->count_down--;
                    continue;
                }
                WaitingTrain waiting_train = {train_track->curr_train->train_id, color, tick};
                Station *next_station = this->next[color * 2 + dir];
                if (next_station->next[color * 2 + dir] == NULL)
                    train_track->curr_train->direction ^= 1; // flip direction
                Platform *next_platform = next_station->platforms[color * 2 + train_track->curr_train->direction];
                omp_set_lock(&next_platform->mutex);
                next_platform->holding_area.emplace(waiting_train);
                omp_unset_lock(&next_platform->mutex);
                train_track->curr_train->train_state = next_station->station_name + "#";
                train_track->curr_train = NULL;
            }
        }
    };

    void run_holdingarea(vector<Train> &train_threads)
    {
        for (int color = 0; color < 3; color++)
        {
            for (int dir = 0; dir < 2; dir++)
            {
                Platform *platform = platforms[color * 2 + dir];
                if (platform == NULL || platform->holding_area.empty())
                    continue;
                if (platform->curr_train == NULL)
                {
                    WaitingTrain waiting_train = platform->holding_area.top();
                    if (waiting_train.color != color)
                        continue;
                    platform->holding_area.pop();
                    platform->count_down = popularity;
                    platform->curr_train = &train_threads[waiting_train.train_id];
                    platform->curr_train->train_state = station_name + "%";
                }
            }
        }
    }

    void run_platform()
    {
        for (int color = 0; color < 3; color++)
        {
            for (int dir = 0; dir < 2; dir++)
            {
                Platform *platform = platforms[color * 2 + dir];
                if (platform == NULL || platform->curr_train == NULL || platform->curr_train->color != color)
                    continue;

                if (platform->count_down > 0)
                {
                    platform->count_down--;
                    continue;
                }
                TrainTrack *train_track = train_tracks[color * 2 + dir];
                if (train_track->curr_train != NULL)
                    continue;
                if (!platform->is_ready)
                {
                    platform->is_ready = true;
                }
                else
                {
                    train_track->count_down = train_track->distance - 1;
                    train_track->curr_train = platform->curr_train;
                    platform->curr_train->train_state = train_track->track_name;
                    platform->curr_train = NULL;
                    platform->is_ready = false;
                }
            }
        }
    }
} Station;
