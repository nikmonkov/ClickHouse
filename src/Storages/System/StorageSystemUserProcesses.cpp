#include <Columns/ColumnArray.h>
#include <Columns/ColumnsNumber.h>
#include <Core/Settings.h>
#include <DataTypes/DataTypeArray.h>
#include <DataTypes/DataTypeMap.h>
#include <DataTypes/DataTypeString.h>
#include <DataTypes/DataTypesNumber.h>
#include <Interpreters/Context.h>
#include <Interpreters/ProcessList.h>
#include <Interpreters/ProfileEventsExt.h>
#include <Storages/System/StorageSystemUserProcesses.h>


namespace DB
{

ColumnsDescription StorageSystemUserProcesses::getColumnsDescription()
{
    auto description = ColumnsDescription
    {
        {"user", std::make_shared<DataTypeString>()},
        {"memory_usage", std::make_shared<DataTypeInt64>()},
        {"peak_memory_usage", std::make_shared<DataTypeInt64>()},
        {"ProfileEvents", std::make_shared<DataTypeMap>(std::make_shared<DataTypeString>(), std::make_shared<DataTypeUInt64>())},
    };

    description.setAliases({
        {"ProfileEvents.Names", {std::make_shared<DataTypeArray>(std::make_shared<DataTypeString>())}, "mapKeys(ProfileEvents)"},
        {"ProfileEvents.Values", {std::make_shared<DataTypeArray>(std::make_shared<DataTypeUInt64>())}, "mapValues(ProfileEvents)"}
    });

    return description;
}

void StorageSystemUserProcesses::fillData(MutableColumns & res_columns, ContextPtr context, const ActionsDAG::Node *, std::vector<UInt8>) const
{
    const auto user_info = context->getProcessList().getUserInfo(true);

    for (const auto & [user, info] : user_info)
    {
        size_t i = 0;

        res_columns[i++]->insert(user);
        res_columns[i++]->insert(info.memory_usage);
        res_columns[i++]->insert(info.peak_memory_usage);
        {
            IColumn * column = res_columns[i++].get();

            if (info.profile_counters)
                ProfileEvents::dumpToMapColumn(*info.profile_counters, column, true);
            else
            {
                column->insertDefault();
            }
        }
    }
}
}
