#include "../common/SQLiteLogger.h"
#include "mspti.h"

int main() {
    msptiActivityKernel k = {MSPTI_ACTIVITY_KIND_KERNEL, 1000, 2000, {0, 1}, 123456789, "Compute", "KernelA"};
    std::vector<msptiActivityKernel> klist;
    klist.push_back(k);
    logger.insertRecords(klist);

    return 0;
}
