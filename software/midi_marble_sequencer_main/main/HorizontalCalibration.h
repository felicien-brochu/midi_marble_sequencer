#include "IRSensBoards.h"
#include "IRSensReader.h"
#include "PushButton.h"
#include "marble_type.h"

typedef enum
{
    HORIZONTAL_CALIBRATION_IDLE,
    HORIZONTAL_CALIBRATION_WAIT_PLACEMENT,
    HORIZONTAL_CALIBRATION_READ,
    HORIZONTAL_CALIBRATION_COMPLETE,
} horizontal_calibration_state_t;

class ColorStatistics
{
public:
    ColorStatistics();

    void push_sample(int value_off, int value_on);

    uint16_t min;
    uint16_t max;
    uint32_t sum;
    uint16_t nb_samples;
    uint16_t mean;
};

class SensorStatistics
{
public:
    SensorStatistics();

    void push_sample(marble_type_t color, int value_off, int value_on);
    void compute_thresholds(uint16_t *thresholds);

    ColorStatistics color_statistics[NUM_MARBLE_TYPE];
};


class HorizontalCalibration
{
public:

    HorizontalCalibration(int marbles_for_one_color, int samples_by_test, int ms_between_samples, int multisampling);

    void update();
    bool is_complete();

private:
    int _marbles_for_one_color;
    int _samples_by_test;
    int _ms_between_samples;
    int _multisampling;

    IRSensBoards _ir_sens_boards;
    IRSensReader _board_reader;
    PushButton _push_button;

    uint32_t *_values_on;
    uint32_t *_values_off;

    horizontal_calibration_state_t _calibration_state;
    uint16_t _measure_count;
    SensorStatistics **_statistics;

    void _idle_state_update();
    void _waiting_placement_state_update();
    void _read_state_update();
    void _print_marble_placement();
    void _print_statistics();
    void _print_csv_data();
    void _print_c_array_thresholds();
    void _print_c_array_means();
    void _print_marble_intervals_for_sensor(uint8_t board_index, uint8_t ir_sens_channel);
    marble_type_t _get_ir_sens_marble_type(uint8_t ir_sens_channel);
};