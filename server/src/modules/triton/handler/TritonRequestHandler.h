#ifndef PROFILER_SERVER_TRITON_REQUEST_HANDLER_H
#define PROFILER_SERVER_TRITON_REQUEST_HANDLER_H
#include "ModuleRequestHandler.h"
#include "ProtocolDefs.h"
namespace Dic::Module::Triton {
class TritonRequestHandler : public ModuleRequestHandler {
public:
    TritonRequestHandler()
    {
        moduleName = MODULE_TRITON;
        async = false;
    }
    ~TritonRequestHandler() override = default;
protected:
    inline static const std::string REQUEST_ERROR_UNKNOWN = "An unknown exception occurred while querying data. Please check whether your data contains anomalies or review the logs for more information.";
};
}
#endif
