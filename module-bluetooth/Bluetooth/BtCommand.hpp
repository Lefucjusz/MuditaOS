// Copyright (c) 2017-2024, Mudita Sp. z.o.o. All rights reserved.
// For licensing, see https://github.com/mudita/MuditaOS/blob/master/LICENSE.md

#pragma once

#include "Result.hpp"
#include <string>

namespace bluetooth
{
    auto set_name(std::string &name) -> Result;
} // namespace bluetooth
