// Copyright (c) 2017-2023, Mudita Sp. z.o.o. All rights reserved.
// For licensing, see https://github.com/mudita/MuditaOS/LICENSE.md

#pragma once

#include <cstdint>

namespace units
{
    using Voltage     = std::uint32_t; /// mV
    using SOC         = std::uint8_t;  /// 0-100%
    using Current     = std::int32_t;  /// mA
    using Temperature = float;
    using Percent     = float;
} // namespace units
