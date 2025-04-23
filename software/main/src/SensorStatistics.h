#include "marble_type.h"

class SensorStatistics
{
public:
    int sensor_channel;
    marble_type_t marble_type;

    unsigned long nb_samples = 0;
    int *diffs;
    double sum_diff = 0;
    double diff0 = -100000000;
    double sum_diff_delta = 0;
    double sum_diff_delta_squared = 0;
    double diff_mean = 0;
    double diff_median = 0;
    double diff_variance = 0;
    double diff_min = 10000000;
    double diff_max = 0;
    double min999 = 10000000;
    double max999 = 0;
    double min9999 = 10000000;
    double max9999 = 0;

    SensorStatistics(int sensor_channel, marble_type_t marble_type, int total_samples);

    void push_sample(int value_off, int value_on);
    void compute_statistics();
    void compute_999_intervals();
    double compute_loss_to_next_marble_type(SensorStatistics *nextStats, double *middle_corrected);
};