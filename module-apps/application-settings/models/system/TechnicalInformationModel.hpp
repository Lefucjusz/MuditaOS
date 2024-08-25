// Copyright (c) 2017-2021, Mudita Sp. z.o.o. All rights reserved.
// For licensing, see https://github.com/mudita/MuditaOS/LICENSE.md

#pragma once

#include "FactoryData.hpp"
#include "TechnicalInformationRepository.hpp"

#include <Application.hpp>
#include <InternalModel.hpp>
#include <ListItemProvider.hpp>

class TechnicalInformationModel : public app::InternalModel<gui::ListItem *>, public gui::ListItemProvider
{
  public:
    explicit TechnicalInformationModel(std::unique_ptr<AbstractFactoryData> &&factoryData,
                                       std::unique_ptr<AbstractTechnicalInformationRepository> &&repository);

    [[nodiscard]] auto requestRecordsCount() -> unsigned override;
    [[nodiscard]] auto getMinimalItemSpaceRequired() const -> unsigned override;
    auto getItem(gui::Order order) -> gui::ListItem * override;
    auto requestRecords(std::uint32_t offset, std::uint32_t limit) -> void override;
    auto requestImei(std::function<void()> onImeiReadCallback) -> void;

    auto createData() -> void;
    auto clearData() -> void;

  private:
    app::ApplicationCommon *application{nullptr};
    std::unique_ptr<AbstractFactoryData> factoryData;
    std::unique_ptr<AbstractTechnicalInformationRepository> technicalInformationRepository;
};
