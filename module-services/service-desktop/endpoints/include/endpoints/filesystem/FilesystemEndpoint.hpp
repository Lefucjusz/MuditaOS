// Copyright (c) 2017-2021, Mudita Sp. z.o.o. All rights reserved.
// For licensing, see https://github.com/mudita/MuditaOS/LICENSE.md

#pragma once

#include <endpoints/Endpoint.hpp>
#include "FilesystemHelper.hpp"
#include <Service/Service.hpp>
#include "FileOperations.hpp"

namespace sdesktop::endpoints
{
    class FilesystemEndpoint : public Endpoint
    {
      public:
        FilesystemEndpoint(sys::Service *ownerServicePtr, FileOperations &fileOperations)
            : Endpoint(ownerServicePtr), helper(std::make_unique<FilesystemHelper>(ownerServicePtr, fileOperations))
        {}

        static auto createInstance(sys::Service *ownerServicePtr) -> std::unique_ptr<Endpoint>
        {
            return std::make_unique<FilesystemEndpoint>(ownerServicePtr, FileOperations::getInstance());
        }

        auto handle(Context &context) -> void override;

      private:
        const std::unique_ptr<FilesystemHelper> helper;
    };
} // namespace sdesktop::endpoints
