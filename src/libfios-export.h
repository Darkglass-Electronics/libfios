// SPDX-FileCopyrightText: 2024 Filipe Coelho <falktx@darkglass.com>
// SPDX-License-Identifier: ISC

#pragma once

#include "libfios.h"

#ifdef __cplusplus
extern "C" {
#endif

FIOS_API
float* new_float_ptr();

FIOS_API
float get_float_ptr_value(float* ptr);

FIOS_API
void delete_float_ptr(float* ptr);

#ifdef __cplusplus
}
#endif
