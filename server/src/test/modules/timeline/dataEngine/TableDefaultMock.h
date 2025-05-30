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
#include "MstxEventsTable.h"
#include "EnumMstxEventTypeTable.h"
#include "ComputeTaskInfoTable.h"
#include "PytorchApiTable.h"
#include "PytorchCallchainsTable.h"
#include "PythonGCTable.h"
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
};
class StringIdsTableMock : public StringIdsTable, public TableDefaultMock {
protected:
    void ExcuteQuery(const std::string &fileId, std::vector<StringIdsPO> &result) override
    {
        StringIdsTable::ExcuteQuery(db, result);
        ClearThreadLocal();
    }
};
class TaskTableMock : public TaskTable, public TableDefaultMock {
protected:
    void ExcuteQuery(const std::string &fileId, std::vector<TaskPO> &result) override
    {
        TaskTable::ExcuteQuery(db, result);
        ClearThreadLocal();
    }
};
class CommucationTaskInfoTableMock : public CommucationTaskInfoTable, public TableDefaultMock {
protected:
    void ExcuteQuery(const std::string &fileId, std::vector<CommucationTaskInfoPO> &result) override
    {
        CommucationTaskInfoTable::ExcuteQuery(db, result);
        ClearThreadLocal();
    }
};
class EnumHcclLinkTypeTableMock : public EnumHcclLinkTypeTable, public TableDefaultMock {
protected:
    void ExcuteQuery(const std::string &fileId, std::vector<EnumHcclLinkTypePO> &result) override
    {
        EnumHcclLinkTypeTable::ExcuteQuery(db, result);
        ClearThreadLocal();
    }
};
class EnumHcclTransportTypeTableMock : public EnumHcclTransportTypeTable, public TableDefaultMock {
protected:
    void ExcuteQuery(const std::string &fileId, std::vector<EnumHcclTransportTypePO> &result) override
    {
        EnumHcclTransportTypeTable::ExcuteQuery(db, result);
        ClearThreadLocal();
    }
};
class EnumHcclRdmaTypeTableMock : public EnumHcclRdmaTypeTable, public TableDefaultMock {
protected:
    void ExcuteQuery(const std::string &fileId, std::vector<EnumHcclRdmaTypePO> &result) override
    {
        EnumHcclRdmaTypeTable::ExcuteQuery(db, result);
        ClearThreadLocal();
    }
};
class EnumHcclDataTypeTableMock : public EnumHcclDataTypeTable, public TableDefaultMock {
protected:
    void ExcuteQuery(const std::string &fileId, std::vector<EnumHcclDataTypePO> &result) override
    {
        EnumHcclDataTypeTable::ExcuteQuery(db, result);
        ClearThreadLocal();
    }
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
class MstxEventsTableMock : public MstxEventsTable, public TableDefaultMock {
protected:
    void ExcuteQuery(const std::string &fileId, std::vector<MstxEventsPO> &result) override
    {
        MstxEventsTable::ExcuteQuery(db, result);
        ClearThreadLocal();
    }
};

class PythonGCTableMock : public PythonGCTable, public TableDefaultMock {
protected:
    void ExcuteQuery(const std::string &fileId, std::vector<PythonGCPO> &result) override
    {
        PythonGCTable::ExcuteQuery(db, result);
        ClearThreadLocal();
    }
};

class EnumMstxEventTypeTableMock : public EnumMstxEventTypeTable, public TableDefaultMock {
protected:
    void ExcuteQuery(const std::string &fileId, std::vector<EnumMstxEventTypePO> &result) override
    {
        EnumMstxEventTypeTable::ExcuteQuery(db, result);
        ClearThreadLocal();
    }
};

class ComputeTaskInfoTableMock : public ComputeTaskInfoTable, public TableDefaultMock {
protected:
    void ExcuteQuery(const std::string &fileId, std::vector<ComputeTaskInfoPO> &result) override
    {
        ComputeTaskInfoTable::ExcuteQuery(db, result);
        ClearThreadLocal();
    }
};

class PytorchApiTableMock : public PytorchApiTable, public TableDefaultMock {
protected:
    void ExcuteQuery(const std::string &fileId, std::vector<PytorchApiPO> &result) override
    {
        PytorchApiTable::ExcuteQuery(db, result);
        ClearThreadLocal();
    }
};


class PytorchCallchainsTableMock : public PytorchCallchainsTable, public TableDefaultMock {
protected:
    void ExcuteQuery(const std::string &fileId, std::vector<PytorchCallchainsPO> &result) override
    {
        PytorchCallchainsTable::ExcuteQuery(db, result);
        ClearThreadLocal();
    }
};


struct PytorchApiDependency {
    std::unique_ptr<StringIdsTableMock> stringIdsTableMock = std::make_unique<StringIdsTableMock>();
    std::unique_ptr<PytorchApiTableMock> pytorchApiTableMock = std::make_unique<PytorchApiTableMock>();
    std::unique_ptr<PytorchCallchainsTableMock> pytorchCallchainsTableMock =
        std::make_unique<PytorchCallchainsTableMock>();
};

struct HardWareDependency {
    std::unique_ptr<StringIdsTableMock> stringIdsTableMock = std::make_unique<StringIdsTableMock>();
    std::unique_ptr<TaskTableMock> taskTableMock = std::make_unique<TaskTableMock>();
    std::unique_ptr<ComputeTaskInfoTableMock> computeTaskInfoTableMock = std::make_unique<ComputeTaskInfoTableMock>();
};

struct CannDependency {
    std::unique_ptr<StringIdsTableMock> stringIdsTableMock = std::make_unique<StringIdsTableMock>();
    std::unique_ptr<CannApiTableMock> cannApiTableMock = std::make_unique<CannApiTableMock>();
    std::unique_ptr<EnumApiTypeTableMock> enumApiTypeTableMock = std::make_unique<EnumApiTypeTableMock>();
};

struct MstxDependency {
    std::unique_ptr<StringIdsTableMock> stringIdsTableMock = std::make_unique<StringIdsTableMock>();
    std::unique_ptr<MstxEventsTableMock> mstxEventsTableMock = std::make_unique<MstxEventsTableMock>();
    std::unique_ptr<EnumMstxEventTypeTableMock> enumMstxEventTypeTableMock =
        std::make_unique<EnumMstxEventTypeTableMock>();
};

struct PythonGcDependency {
        std::unique_ptr<PythonGCTableMock> tableMock = std::make_unique<PythonGCTableMock>();
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
