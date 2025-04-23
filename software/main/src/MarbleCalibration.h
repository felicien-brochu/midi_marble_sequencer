#include "IRSensBoard.h"
#include "IRSensBoardReaderOneShot.h"
#include "PushButton.h"
#include "marble_type.h"
#include "SensorStatistics.h"

typedef enum
{
    MARBLE_CALIBRATION_IDLE,
    MARBLE_CALIBRATION_WAIT_PLACEMENT,
    MARBLE_CALIBRATION_READ,
    MARBLE_CALIBRATION_COMPLETE,
} marble_calibration_state_t;



class MarbleCalibration
{
public:

    MarbleCalibration(int marble_type_shifts, int samples_by_test, int ms_between_samples, int multisampling);

    void update();
    bool is_complete();

private:

    int _marble_type_shifts;
    int _samples_by_test;
    int _ms_between_samples;
    int _multisampling;

    IRSensBoard _ir_sens_board;
    IRSensBoardReaderOneShot _board_reader;
    PushButton _push_button;

    int *_values_on;
    int *_values_off;

    marble_calibration_state_t _calibration_state;
    marble_type_t _first_marble_type;
    int _measure_count;
    SensorStatistics **_statistics;

    void _idle_state_update();
    void _waiting_placement_state_update();
    void _read_state_update();
    void _shift_marble_types();
    void _print_marble_placement();
    void _print_statistics();
    void _print_marble_intervals_for_sensor(uint8_t ir_sens_channel);
    marble_type_t _get_ir_sens_marble_type(uint8_t ir_sens_channel);
};