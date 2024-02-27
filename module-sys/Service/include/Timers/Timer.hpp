// Copyright (c) 2017-2021, Mudita Sp. z.o.o. All rights reserved.
// For licensing, see https://github.com/mudita/MuditaOS/LICENSE.md

#pragma once

#include <chrono>
#include <functional>

namespace sys
{
    class Timer;

    namespace timer
    {
        using TimerCallback                   = std::function<void(Timer &)>;
        inline constexpr auto InfiniteTimeout = std::chrono::milliseconds::max();
        enum class Type
        {
            Periodic,
            SingleShot
        };
    } // namespace timer

    class Timer
    {
      public:
        virtual ~Timer() noexcept                                   = default;
        virtual void start()                                        = 0;
        virtual void restart(std::chrono::milliseconds newInterval) = 0;
        virtual void stop()                                         = 0;
        [[nodiscard]] virtual bool isActive() const noexcept        = 0;
    };
} // namespace sys
