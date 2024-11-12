/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */

#ifndef PROFILER_SERVER_TABLEDEFAULTMOCK_H
#define PROFILER_SERVER_TABLEDEFAULTMOCK_H
#include "EnumApiTypeTable.h"
#include "CannApiTable.h"
#include "CommucationOpTable.h"
#include "TaskTable.h"
#include "CommucationTaskInfoTable.h"
#include "EnumHcclLinkTypeTable.h"
#include "EnumHcclTransportTypeTable.h"
#include "EnumHcclRdmaTypeTable.h"
#include "EnumHcclDataTypeTable.h"

using namespace Dic::Module::Timeline;
namespace Dic::TimeLine::Table::Default::Mock {
class TableDefaultMock {
public:
    void SetDb(sqlite3 *dbPtr)
    {
        db = dbPtr;
    }

protected:
    sqlite3 *db = nullptr;
};
class CommucationOpTableMock : public CommucationOpTable, public TableDefaultMock {
protected:
    void ExcuteQuery(const std::string &fileId, std::vector<CommucationTaskOpPO> &result) override
    {
        CommucationOpTable::ExcuteQuery(db, result);
        ClearThreadLocal();
    }

private:
    sqlite3 *db = nullptr;
};
class StringIdsTableMock : public StringIdsTable, public TableDefaultMock {
protected:
    void ExcuteQuery(const std::string &fileId, std::vector<StringIdsPO> &result) override
    {
        StringIdsTable::ExcuteQuery(db, result);
        ClearThreadLocal();
    }

private:
    sqlite3 *db = nullptr;
};
class TaskTableMock : public TaskTable, public TableDefaultMock {
protected:
    void ExcuteQuery(const std::string &fileId, std::vector<TaskPO> &result) override
    {
        TaskTable::ExcuteQuery(db, result);
        ClearThreadLocal();
    }

private:
    sqlite3 *db = nullptr;
};
class CommucationTaskInfoTableMock : public CommucationTaskInfoTable, public TableDefaultMock {
protected:
    void ExcuteQuery(const std::string &fileId, std::vector<CommucationTaskInfoPO> &result) override
    {
        CommucationTaskInfoTable::ExcuteQuery(db, result);
        ClearThreadLocal();
    }

private:
    sqlite3 *db = nullptr;
};
class EnumHcclLinkTypeTableMock : public EnumHcclLinkTypeTable, public TableDefaultMock {
protected:
    void ExcuteQuery(const std::string &fileId, std::vector<EnumHcclLinkTypePO> &result) override
    {
        EnumHcclLinkTypeTable::ExcuteQuery(db, result);
        ClearThreadLocal();
    }

private:
    sqlite3 *db = nullptr;
};
class EnumHcclTransportTypeTableMock : public EnumHcclTransportTypeTable, public TableDefaultMock {
protected:
    void ExcuteQuery(const std::string &fileId, std::vector<EnumHcclTransportTypePO> &result) override
    {
        EnumHcclTransportTypeTable::ExcuteQuery(db, result);
        ClearThreadLocal();
    }

private:
    sqlite3 *db = nullptr;
};
class EnumHcclRdmaTypeTableMock : public EnumHcclRdmaTypeTable, public TableDefaultMock {
protected:
    void ExcuteQuery(const std::string &fileId, std::vector<EnumHcclRdmaTypePO> &result) override
    {
        EnumHcclRdmaTypeTable::ExcuteQuery(db, result);
        ClearThreadLocal();
    }

private:
    sqlite3 *db = nullptr;
};
class EnumHcclDataTypeTableMock : public EnumHcclDataTypeTable, public TableDefaultMock {
protected:
    void ExcuteQuery(const std::string &fileId, std::vector<EnumHcclDataTypePO> &result) override
    {
        EnumHcclDataTypeTable::ExcuteQuery(db, result);
        ClearThreadLocal();
    }

private:
    sqlite3 *db = nullptr;
};
class EnumApiTypeTableMock : public EnumApiTypeTable, public TableDefaultMock {
protected:
    void ExcuteQuery(const std::string &fileId, std::vector<EnumApiTypePO> &result) override
    {
        EnumApiTypeTable::ExcuteQuery(db, result);
        ClearThreadLocal();
    }
};
class CannApiTableMock : public CannApiTable, public TableDefaultMock {
protected:
    void ExcuteQuery(const std::string &fileId, std::vector<CannApiPO> &result) override
    {
        CannApiTable::ExcuteQuery(db, result);
        ClearThreadLocal();
    }
};


struct HcclDependency {
    std::unique_ptr<CommucationOpTableMock> commucationOpTableMock = std::make_unique<CommucationOpTableMock>();
    std::unique_ptr<StringIdsTableMock> stringIdsTableMock = std::make_unique<StringIdsTableMock>();
    std::unique_ptr<TaskTableMock> taskTableMock = std::make_unique<TaskTableMock>();
    std::unique_ptr<CommucationTaskInfoTableMock> commucationTaskInfoTableMock =
        std::make_unique<CommucationTaskInfoTableMock>();
    std::unique_ptr<EnumHcclLinkTypeTableMock> enumHcclLinkTypeTableMock =
        std::make_unique<EnumHcclLinkTypeTableMock>();
    std::unique_ptr<EnumHcclTransportTypeTableMock> enumHcclTransportTypeTableMock =
        std::make_unique<EnumHcclTransportTypeTableMock>();
    std::unique_ptr<EnumHcclRdmaTypeTableMock> enumHcclRdmaTypeTableMock =
        std::make_unique<EnumHcclRdmaTypeTableMock>();
    std::unique_ptr<EnumHcclDataTypeTableMock> enumHcclDataTypeTableMock =
        std::make_unique<EnumHcclDataTypeTableMock>();
};
}


#endif // PROFILER_SERVER_TABLEDEFAULTMOCK_H
