// Copyright (c) 2017-2024, Mudita Sp. z.o.o. All rights reserved.
// For licensing, see https://github.com/mudita/MuditaOS/blob/master/LICENSE.md

#pragma once

namespace purefs::fs
{
    //! Mount flags struct
    struct mount_flags
    {
        enum _mount_flags
        {
            read_only = 1,
            remount   = 32,
        };
    };
} // namespace purefs::fs
