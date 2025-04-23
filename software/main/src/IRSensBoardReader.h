#pragma once

#include "IRSensBoard.h"

class IRSensBoardReader
{
public:
    virtual void read_values(int *values_off, int *values_on, int multisampling = 1) = 0;
};