#include "SensorStatistics.h"

#include <stdio.h>

SensorStatistics::SensorStatistics(int sensor_channel, marble_type_t marble_type, int total_samples)
{
    this->sensor_channel = sensor_channel;
    this->marble_type = marble_type;
    diffs = (int *) malloc(total_samples * sizeof(int));
}

void SensorStatistics::push_sample(int value_off, int value_on)
{
    nb_samples++;


    int diff = value_off - value_on;

    diffs[nb_samples - 1] = diff;


    sum_diff += diff;

    diff_mean = sum_diff / nb_samples;

    if (nb_samples == 0)
    {
        diff0 = diff;
    }

    sum_diff_delta += diff - diff0;
    sum_diff_delta_squared += (diff - diff0) * (diff - diff0);

    if (nb_samples > 1)
    {
        diff_variance = (sum_diff_delta_squared - ((sum_diff_delta * sum_diff_delta) / nb_samples)) / (nb_samples - 1);
    }

    if (diff_min > diff)
    {
        diff_min = diff;
    }

    if (diff_max < diff)
    {
        diff_max = diff;
    }
}

void SensorStatistics::compute_statistics()
{
    compute_999_intervals();
}


void swap(int *a, int *b)
{
    int t = *a;
    *a = *b;
    *b = t;
}

int partition(int arr[], int l, int h)
{
    int x = arr[h];
    int i = (l - 1);

    for (int j = l; j <= h - 1; j++)
    {
        if (arr[j] <= x)
        {
            i++;
            swap(&arr[i], &arr[j]);
        }
    }
    swap(&arr[i + 1], &arr[h]);
    return (i + 1);
}

int delta_median(int value, int median)
{
    return abs(median - value);
}

int partition_delta_median(int arr[], int l, int h, int median)
{
    int x = delta_median(arr[h], median);
    int i = (l - 1);

    for (int j = l; j <= h - 1; j++)
    {
        if (delta_median(arr[j], median) <= x)
        {
            i++;
            swap(&arr[i], &arr[j]);
        }
    }
    swap(&arr[i + 1], &arr[h]);
    return (i + 1);
}

/* A[] --> Array to be sorted,
l --> Starting index,
h --> Ending index */
void quick_sort(int arr[], int l, int h, int median, bool delta_median)
{
    // Create an auxiliary stack
    int stack[h - l + 1];

    // initialize top of stack
    int top = -1;

    // push initial values of l and h to stack
    stack[++top] = l;
    stack[++top] = h;

    // Keep popping from stack while is not empty
    while (top >= 0)
    {
        // Pop h and l
        h = stack[top--];
        l = stack[top--];

        // Set pivot element at its correct position
        // in sorted array
        int p;
        if (delta_median)
        {
            p = partition_delta_median(arr, l, h, median);
        }
        else
        {
            p = partition(arr, l, h);
        }

        // If there are elements on left side of pivot,
        // then push left side to stack
        if (p - 1 > l)
        {
            stack[++top] = l;
            stack[++top] = p - 1;
        }

        // If there are elements on right side of pivot,
        // then push right side to stack
        if (p + 1 < h)
        {
            stack[++top] = p + 1;
            stack[++top] = h;
        }
    }
}



void SensorStatistics::compute_999_intervals()
{
    quick_sort(diffs, 0, nb_samples - 1, 0, false);
    diff_median = (diffs[nb_samples / 2] + diffs[nb_samples / 2 + 1]) / 2;

    quick_sort(diffs, 0, nb_samples - 1, diff_median, true);

    double nb_samples_999 = (double) nb_samples / 1000.;
    double nb_samples_9999 = (double) nb_samples / 10000.;

    for (int i = nb_samples - 1; i >= 0; i--)
    {
        if (i <= nb_samples - nb_samples_9999)
        {
            if (min9999 > diffs[i])
            {
                min9999 = diffs[i];
            }

            if (max9999 < diffs[i])
            {
                max9999 = diffs[i];
            }
        }

        if (i <= nb_samples - nb_samples_999)
        {
            if (min999 > diffs[i])
            {
                min999 = diffs[i];
            }

            if (max999 < diffs[i])
            {
                max999 = diffs[i];
            }
        }
    }
}

double SensorStatistics::compute_loss_to_next_marble_type(SensorStatistics *nextStats, double *middle_corrected) {
    quick_sort(diffs, 0, nb_samples - 1, 0, false);
    quick_sort(nextStats->diffs, 0, nb_samples - 1, 0, false);

    int i;
    for (i = 0; i < nb_samples; i++) {
        if (diffs[nb_samples - 1 - i] < nextStats->diffs[i]) {
            break;
        }
    }

    *middle_corrected = (diffs[nb_samples - 1 - i] + nextStats->diffs[i]) / 2;

    return (double) (i * 2) / (double) nb_samples;
}
