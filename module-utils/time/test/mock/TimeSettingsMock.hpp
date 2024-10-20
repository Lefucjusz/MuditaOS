// Copyright (c) 2017-2024, Mudita Sp. z.o.o. All rights reserved.
// For licensing, see https://github.com/mudita/MuditaOS/blob/master/LICENSE.md

#pragma once

#include "time/time_conversion.hpp"

class BaseTimeSettings : public utils::time::TimeSettingsInterface
{
  public:
    virtual bool isTimeFormat12h() const
    {
        return true;
    };
    virtual bool isDateFormatDDMM() const
    {
        return true;
    };
};