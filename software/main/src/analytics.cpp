#include "analytics.h"

#include <esp_log.h>
#include <stdio.h>
#include <math.h>
#include <esp_timer.h>

void print_values(int *values_off, int *values_on, uint8_t ir_sens_on_board)
{
    for (int i = 0; i < ir_sens_on_board; i++)
    {
        // printf(">on%d: %d\n", i, values_on[i]);
        // printf(">off%d: %d\n", i, values_off[i]);
        printf(">diff%d: %d\n", i, values_off[i] - values_on[i]);
    }
}

void statistics(int *values_off, int *values_on, IRSensBoard *ir_sens_board, uint64_t print_every_seconds) {
    static uint64_t call_count = 0;
    static double *sum_off = NULL;
    static double *sum_on = NULL;
    static double *sum_diff = NULL;
    static double *sum_squared_diff = NULL;
    static double *diffs0 = NULL;
    static double *sum_diff_delta = NULL;
    static double *sum_diff_delta_squared = NULL;
    static double *diff_means = NULL;
    static double *diff_variances = NULL;
    static double *diff_variances2 = NULL;
    static double *diff_mins = NULL;
    static double *diff_maxs = NULL;
    static double diff_max = 0;
    static double diff_min = 10000000;
    static double diff_mean = 0;
    static uint64_t last_print_time = esp_timer_get_time();

    if (call_count == 0)
    {
        sum_off = (double*) malloc(ir_sens_board->ir_sens_on_board * sizeof(double));
        sum_on = (double *)malloc(ir_sens_board->ir_sens_on_board * sizeof(double));
        sum_diff = (double *)malloc(ir_sens_board->ir_sens_on_board * sizeof(double));
        sum_squared_diff = (double *)malloc(ir_sens_board->ir_sens_on_board * sizeof(double));
        diffs0 = (double *)malloc(ir_sens_board->ir_sens_on_board * sizeof(double));
        sum_diff_delta = (double *)malloc(ir_sens_board->ir_sens_on_board * sizeof(double));
        sum_diff_delta_squared = (double *)malloc(ir_sens_board->ir_sens_on_board * sizeof(double));
        diff_means = (double *)malloc(ir_sens_board->ir_sens_on_board * sizeof(double));
        diff_variances = (double *)malloc(ir_sens_board->ir_sens_on_board * sizeof(double));
        diff_variances2 = (double *)malloc(ir_sens_board->ir_sens_on_board * sizeof(double));
        diff_mins = (double *)malloc(ir_sens_board->ir_sens_on_board * sizeof(double));
        diff_maxs = (double *)malloc(ir_sens_board->ir_sens_on_board * sizeof(double));

        for (int i = 0; i < ir_sens_board->ir_sens_on_board; i++)
        {
            sum_off[i] = 0;
            sum_on[i] = 0;
            sum_diff[i] = 0;
            sum_squared_diff[i] = 0;
            diffs0[i] = values_off[i] - values_on[i];
            sum_diff_delta[i] = 0;
            sum_diff_delta_squared[i] = 0;
            diff_means[i] = 0;
            diff_variances[i] = 0;
            diff_variances2[i] = 0;
            diff_mins[i] = 10000000;
            diff_maxs[i] = 0;
        }
    }

    call_count++;

    double diff_means_sum = 0;

    for (int i = 0; i < ir_sens_board->ir_sens_on_board; i++)
    {
        sum_off[i] += values_off[i];
        sum_on[i] += values_on[i];

        double diff = values_off[i] - values_on[i];
        sum_diff[i] += diff;
        sum_squared_diff[i] += diff * diff;

        diff_means[i] = sum_diff[i] / call_count;
        diff_means_sum += diff_means[i];

        if (call_count > 1)
        {
            diff_variances[i] = (sum_squared_diff[i] - (sum_diff[i] * sum_diff[i]) / call_count) / (call_count - 1);
        }

        sum_diff_delta[i] += diff - diffs0[i];
        sum_diff_delta_squared[i] += (diff - diffs0[i]) * (diff - diffs0[i]);
        if (call_count > 1)
        {
            diff_variances2[i] = (sum_diff_delta_squared[i] - ((sum_diff_delta[i] * sum_diff_delta[i]) / call_count)) / (call_count - 1);
        }

        if (diff_mins[i] > diff)
        {
            diff_mins[i] = diff;

            if (diff_min > diff) {
                diff_min = diff;
            }
        }

        if (diff_maxs[i] < diff) {
            diff_maxs[i] = diff;

            if (diff_max < diff) {
                diff_max = diff;
            }
        }
    }

    diff_mean = diff_means_sum / ir_sens_board->ir_sens_on_board ;

    if (esp_timer_get_time() - last_print_time >= print_every_seconds * 1000000) {
        last_print_time = esp_timer_get_time();
        double super_sum_squared = 0;
        double super_sum = 0;
        double n_values = call_count * ir_sens_board->ir_sens_on_board;

        printf("+----------+----------+----------+----------+----------+----------+----------+\n");
        printf("|  sensor  |   mean   | ecartype | -3sigma  |   min    | +3sigma  |   max    |\n");

        for (int i = 0; i < ir_sens_board->ir_sens_on_board; i++)
        {
            printf("|  sens%2d  |  %6.0f  |  %6.0f  |  %6.0f  |  %6.0f  |  %6.0f  |  %6.0f  |\n", ir_sens_board->get_sensor_channel(i), diff_means[i], sqrt(diff_variances2[i]), diff_means[i] - 3 * sqrt(diff_variances2[i]), diff_mins[i], diff_means[i] + 3 * sqrt(diff_variances2[i]), diff_maxs[i]);

            super_sum += sum_diff[i];
            super_sum_squared += sum_squared_diff[i];
        }

        double super_variance = (super_sum_squared - (super_sum * super_sum) / n_values) / (n_values - 1);

        printf("+----------+----------+----------+----------+----------+----------+----------+\n");

        printf("mean   : %6.0f\n", diff_mean);
        printf("sigma  : %6.0f\n", sqrt(super_variance));
        printf("-3sigma: %6.0f\n", diff_mean - 3 * sqrt(super_variance));
        printf("min    : %6.0f\n", diff_min);
        printf("+3sigma: %6.0f\n", diff_mean + 3 * sqrt(super_variance));
        printf("max    : %6.0f\n", diff_max);
        printf("values read: %6llu\n", call_count);
        printf("\n");
    }
}

void distribution(int *values_off, int *values_on, uint8_t sensor_index, IRSensBoard *ir_sens_board, uint64_t print_every_seconds)
{
    static uint64_t call_count = 0;
    static uint64_t last_print_time = esp_timer_get_time();
    static uint32_t buckets[512];

    uint32_t value = (uint32_t)abs(values_off[sensor_index] - values_on[sensor_index]);

    if (call_count == 0)
    {
        for (int i = 0; i < 512; i++) {
            buckets[i] = 0;
        }
    }

    call_count++;

    buckets[value / 8]++;

    if (esp_timer_get_time() - last_print_time >= print_every_seconds * 1000000)
    {
        last_print_time = esp_timer_get_time();

        int min_non_zero = -1;
        int max_non_zero = -1;

        for (int i = 0; i < 512; i++)
        {
            if (buckets[i] > 0 && min_non_zero == -1) {
                min_non_zero = i;
                break;
            }
        }
        for (int i = 512 - 1; i >= 0 ; i--)
        {
            if (buckets[i] > 0 && max_non_zero == -1)
            {
                max_non_zero = i;
                break;
            }
        }

        const int lines = 30;
        const int columns = 64;
        static char *line = (char *) malloc(columns * sizeof(char) + 2);

        uint32_t graph_buckets[columns];
        uint32_t max_in_bucket = 0;

        for (int i = 0; i < columns; i++)
        {
            graph_buckets[i] = 0;
        }

        for (int i = min_non_zero; i <= max_non_zero; i++) {
            int index = (int) ((((double)i - (double)min_non_zero) / ((double)max_non_zero - (double)min_non_zero)) * columns);
            graph_buckets[index] += buckets[i];
        }

        for (int i = 0; i < columns; i++)
        {
            if (graph_buckets[i] > max_in_bucket) {
                max_in_bucket = graph_buckets[i];
            }
        }

        const double value_divisor = max_in_bucket / lines;
        line[columns] = '\n';
        line[columns + 1] = '\0';

        for (int i = 0; i < lines; i++)
        {
            for (int j = 0; j < columns; j++)
            {
                line[j] = (graph_buckets[j] / value_divisor) > (lines - i - 1) ? '=' : ' ';
            }

            printf(line);
        }

        printf("min: %d\n", min_non_zero * 8);
        printf("max: %d\n", max_non_zero * 8);
        printf("max_in_bucket: %lu\n", max_in_bucket);
    }
}