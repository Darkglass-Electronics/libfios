// SPDX-FileCopyrightText: 2024 Filipe Coelho <falktx@darkglass.com>
// SPDX-License-Identifier: ISC

%module "libfios"
%{
#include "libfios.h"
#include "libfios-export.h"
%}
%include "libfios.h"
%include "libfios-export.h"
