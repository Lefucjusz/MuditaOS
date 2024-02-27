// Copyright (c) 2017-2021, Mudita Sp. z.o.o. All rights reserved.
// For licensing, see https://github.com/mudita/MuditaOS/LICENSE.md

#pragma once

#include "Table.hpp"
#include "Record.hpp"
#include "utf8/UTF8.hpp"

struct ContactsAddressTableRow : public Record
{
    std::uint32_t contactID = DB_ID_NONE;
    UTF8 address;
    UTF8 note;
    UTF8 mail;
};

enum class ContactAddressTableFields
{
    Mail
};

class ContactsAddressTable : public Table<ContactsAddressTableRow, ContactAddressTableFields>
{
  public:
    explicit ContactsAddressTable(Database *db);

    virtual ~ContactsAddressTable() = default;

    bool create() override final;
    bool add(ContactsAddressTableRow entry) override final;
    bool removeById(uint32_t id) override final;
    bool update(ContactsAddressTableRow entry) override final;

    ContactsAddressTableRow getById(uint32_t id) override final;
    std::vector<ContactsAddressTableRow> getLimitOffset(std::uint32_t offset, std::uint32_t limit) override final;
    std::vector<ContactsAddressTableRow> getLimitOffsetByField(std::uint32_t offset,
                                                               std::uint32_t limit,
                                                               ContactAddressTableFields field,
                                                               const char *str) override final;

    std::uint32_t count() override final;
    std::uint32_t countByFieldId(const char *field, std::uint32_t id) override final;
};
