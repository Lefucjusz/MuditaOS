// Copyright (c) 2017-2023, Mudita Sp. z.o.o. All rights reserved.
// For licensing, see https://github.com/mudita/MuditaOS/LICENSE.md

#include "TechnicalInformationRepository.hpp"
#include <service-cellular/ServiceCellular.hpp>
#include <EventStore.hpp>

TechnicalInformationRepository::TechnicalInformationRepository(app::ApplicationCommon *application)
    : app::AsyncCallbackReceiver{application}, application{application}
{}

void TechnicalInformationRepository::readImei(AbstractTechnicalInformationRepository::OnReadCallback readDoneCallback)
{

    auto callback = [this](const std::string &imei) { imeiStr = imei; };

    auto msg  = std::make_unique<cellular::GetImeiRequest>();
    auto task = app::AsyncRequest::createFromMessage(std::move(msg), ::service::name::cellular);
    auto cb   = [callback, readDoneCallback](auto response) {
        auto result = dynamic_cast<cellular::GetImeiResponse *>(response);
        if (result != nullptr && result->retCode == sys::ReturnCodes::Success) {
            callback(*result->getImei());
        }
        readDoneCallback();
        return true;
    };
    task->execute(this->application, this, cb);
}

std::string TechnicalInformationRepository::getImei() const
{
    return imeiStr;
}

std::string TechnicalInformationRepository::getBatteryLevel() const
{
    return std::to_string(Store::Battery::get().level) + "%";
}
