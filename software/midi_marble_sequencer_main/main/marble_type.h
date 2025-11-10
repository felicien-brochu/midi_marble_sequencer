#pragma once

// Acrylic marbles
typedef enum
{
    NO_MARBLE = 0,
    BROWN_MARBLE,
    ORANGE_MARBLE,
    // RED_MARBLE,
    GREEN_MARBLE,
    BLUE_MARBLE,
    WHITE_MARBLE,
    NUM_MARBLE_TYPE,
} marble_type_t;

const char *marble_type_to_string(marble_type_t marble_type);