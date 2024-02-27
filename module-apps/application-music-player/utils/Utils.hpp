#pragma once

#include <string>
#include <time/time_constants.hpp>

namespace utils
{
    inline std::string createMinutesSecondsString(std::uint32_t seconds)
    {
        const auto minutes          = seconds / time::secondsInMinute;
        const auto secondsRemainder = seconds % time::secondsInMinute;
        return std::to_string(minutes) + "m " + std::to_string(secondsRemainder) + "s";
    }
} // namespace utils
