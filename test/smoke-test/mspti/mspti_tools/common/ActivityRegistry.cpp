#include "ActivityRegistry.h"

std::vector<TableInfo>& getRegistry() {
    static std::vector<TableInfo> registry;
    return registry;
}
