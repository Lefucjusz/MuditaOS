// Copyright (c) 2017-2023, Mudita Sp. z.o.o. All rights reserved.
// For licensing, see https://github.com/mudita/MuditaOS/LICENSE.md

#include "Common.hpp"
#include <service-fileindexer/StartupIndexer.hpp>
#include <service-fileindexer/ServiceFileIndexerName.hpp>

#include <Timers/TimerFactory.hpp>
#include <purefs/filesystem_paths.hpp>
#include <purefs/fs/inotify_message.hpp>
#include <log/log.hpp>

#include <fstream>
#include <queries/multimedia_files/QueryMultimediaFilesRemove.hpp>
#include <service-db/DBServiceAPI.hpp>

namespace service::detail
{
    using namespace std::literals;
    using namespace std::chrono_literals;

    const auto lockFilePath = purefs::dir::getSystemVarDirPath() / ".directory_is_indexed";

    constexpr auto indexingInterval = 50ms;
    constexpr auto startupDelay     = 10000ms;

    bool isDirectoryFullyTraversed(const std::filesystem::recursive_directory_iterator &directory)
    {
        return directory == std::filesystem::recursive_directory_iterator();
    }

    bool createLockFile()
    {
        std::ofstream ofs(lockFilePath);
        ofs << time(nullptr);
        return ofs.good();
    }

    bool hasLockFile()
    {
        std::error_code errorCode;
        return std::filesystem::is_regular_file(lockFilePath, errorCode);
    }

    bool removeLockFile()
    {
        if (hasLockFile()) {
            std::error_code errorCode;
            if (!remove(lockFilePath, errorCode)) {
                LOG_ERROR("Failed to remove lock file, error %d", errorCode.value());
                return false;
            }
        }
        return true;
    }

    std::optional<std::filesystem::recursive_directory_iterator> scanPath(std::vector<std::string>::const_iterator path)
    {
        std::error_code errorCode;
        auto mSubDirIterator = std::filesystem::recursive_directory_iterator(*path, errorCode);
        if (errorCode) {
            LOG_WARN("Directory '%s' not indexed, it does not exist", path->c_str());
            return std::nullopt;
        }
        return mSubDirIterator;
    }

    StartupIndexer::StartupIndexer(const std::vector<std::string> &paths) : directoriesToScan{paths}
    {}

    // Process single entry
    auto StartupIndexer::processEntry(std::shared_ptr<sys::Service> svc,
                                      const std::filesystem::recursive_directory_iterator::value_type &entry) -> void
    {
        if (std::filesystem::is_regular_file(entry)) {
            const auto &extension = std::filesystem::path(entry).extension();
            if (!isExtensionSupported(extension)) {
                LOG_WARN("Unsupported extension '%s'", extension.c_str());
                return;
            }

            const auto &absPath = std::filesystem::absolute(entry).string();
            auto inotifyMsg =
                std::make_shared<purefs::fs::message::inotify>(purefs::fs::inotify_flags::close_write, absPath, ""sv);
            svc->bus.sendUnicast(std::move(inotifyMsg), std::string(service::name::file_indexer));
        }
    }

    auto StartupIndexer::onTimerTimeout(std::shared_ptr<sys::Service> svc) -> void
    {
        if (mForceStop) {
            return;
        }
        if (isDirectoryFullyTraversed(mSubDirIterator)) {
            if (mTopDirIterator == std::cend(directoriesToScan)) {
                createLockFile();
                mIdxTimer.stop();
                LOG_INFO("Initial startup indexing finished");
                return;
            }
            else {
                if (const auto &result = scanPath(mTopDirIterator)) {
                    mSubDirIterator = *result;
                }
                mTopDirIterator++;
            }
        }
        else {
            processEntry(std::move(svc), *mSubDirIterator);
            mSubDirIterator++;
        }

        mIdxTimer.restart(indexingInterval);
    }

    // Setup timers for notification
    auto StartupIndexer::setupTimers(std::shared_ptr<sys::Service> svc, std::string_view svc_name) -> void
    {
        mIdxTimer = sys::TimerFactory::createSingleShotTimer(
            svc.get(), svc_name.data(), startupDelay, [this, svc]([[maybe_unused]] sys::Timer &t) {
                onTimerTimeout(svc);
            });
        mIdxTimer.start();
    }

    // Start the initial file indexing
    auto StartupIndexer::start(std::shared_ptr<sys::Service> svc, std::string_view svc_name) -> void
    {
        if (!hasLockFile()) {
            LOG_INFO("Initial startup indexing started");

            auto query = std::make_unique<db::multimedia_files::query::RemoveAll>();
            DBServiceAPI::GetQuery(svc.get(), db::Interface::Name::MultimediaFiles, std::move(query));

            mTopDirIterator = std::begin(directoriesToScan);
            setupTimers(svc, svc_name);
            mForceStop = false;
        }
        else {
            LOG_INFO("Initial startup indexing not required - already indexed");
        }
    }

    void StartupIndexer::stop()
    {
        mForceStop = true;
        mIdxTimer.stop();
    }

    void StartupIndexer::reset()
    {
        stop();
        removeLockFile();
    }
} // namespace service::detail
