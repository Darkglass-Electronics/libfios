// SPDX-FileCopyrightText: 2024 Filipe Coelho <falktx@darkglass.com>
// SPDX-License-Identifier: ISC

#include "libfios-export.h"

#include <stdlib.h>

float* new_float_ptr()
{
    return calloc(1, sizeof(float));
}

float get_float_ptr_value(float* ptr)
{
    return *ptr;
}

void delete_float_ptr(float* ptr)
{
    free(ptr);
}
