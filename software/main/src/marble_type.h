#pragma once

// // Glass marbles
// typedef enum
// {
//     NO_MARBLE = 0,
//     BLACK_MARBLE,
//     BLUE_MARBLE,
//     YELLOW_MARBLE,
//     WHITE_MARBLE,
// } marble_type_t;

// Acrylic marbles
typedef enum
{
    NO_MARBLE = 0,
    BLACK_MARBLE,
    RED_MARBLE,
    BLUE_MARBLE,
    YELLOW_MARBLE,
    WHITE_MARBLE,
} marble_type_t;

const char *marble_type_to_string(marble_type_t marble_type);