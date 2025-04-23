#include "marble_type.h"

const char *marble_type_to_string(marble_type_t marble_type)
{
    switch (marble_type)
    {
    case NO_MARBLE:
        return " NONE   ";
    case BLACK_MARBLE:
        return " Black  ";
    case RED_MARBLE:
        return " Red    ";
    case BLUE_MARBLE:
        return " Blue   ";
    case YELLOW_MARBLE:
        return " Yellow ";
    case WHITE_MARBLE:
        return " White  ";
    default:
        return "  ####  ";
    }
}