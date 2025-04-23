#include <soc/gpio_num.h>

typedef enum
{
    PUSH_BUTTON_UP,
    PUSH_BUTTON_DOWN,
} push_button_state_t;


class PushButton
{
public:
    PushButton(gpio_num_t gpio_num, bool one_is_up);

    void update();

    bool is_up();
    bool is_down();

    void start_listening_clicks();
    void stop_listening_clicks();
    bool has_click_listener();

    bool has_click_event_pending();
    void click_event_accounted_for();

private:
    gpio_num_t _gpio_num;
    bool _one_is_up;

    push_button_state_t _push_button_state;
    
    bool _has_click_listener;
    bool _click_event_pending;


    push_button_state_t _get_current_state();
};