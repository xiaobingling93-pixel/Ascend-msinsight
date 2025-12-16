#include <atomic>
#include <iostream>
#include <vector>
#include <thread>
#include "acl/acl.h"
#include "acl/acl_prof.h"
#include "aclnnop/aclnn_add.h"

// MSPTI
#include "mspti/mspti.h"

// mstx
#include "mstx/ms_tools_ext.h"

#define CHECK_RET(cond, return_expr) \
    do {                               \
        if (!(cond)) {                   \
        return_expr;                   \
        }                                \
    } while (0)

#define LOG_PRINT(message, ...)     \
    do {                              \
        printf(message, ##__VA_ARGS__); \
    } while (0)

#define ALIGN_SIZE (8)
#define ALIGN_BUFFER(buffer, align)                                                 \
  (((uintptr_t) (buffer) & ((align)-1)) ? ((buffer) + (align) - ((uintptr_t) (buffer) & ((align)-1))) : (buffer))

int64_t GetShapeSize(const std::vector<int64_t>& shape) {
    int64_t shapeSize = 1;
    for (auto i : shape) {
        shapeSize *= i;
  }
    return shapeSize;
}

int Init(int32_t deviceId, aclrtContext* context, aclrtStream* stream) {
    // 固定写法，acl初始化
    auto ret = aclrtSetDevice(deviceId);
    CHECK_RET(ret == ACL_SUCCESS, LOG_PRINT("aclrtSetDevice failed. ERROR: %d\n", ret); return ret);
    ret = aclrtCreateContext(context, deviceId);
    CHECK_RET(ret == ACL_SUCCESS, LOG_PRINT("aclrtCreateContext failed. ERROR: %d\n", ret); return ret);
    ret = aclrtSetCurrentContext(*context);
    CHECK_RET(ret == ACL_SUCCESS, LOG_PRINT("aclrtSetCurrentContext failed. ERROR: %d\n", ret); return ret);
    ret = aclrtCreateStream(stream);
    CHECK_RET(ret == ACL_SUCCESS, LOG_PRINT("aclrtCreateStream failed. ERROR: %d\n", ret); return ret);

    ret = aclInit(nullptr);
    CHECK_RET(ret == ACL_SUCCESS, LOG_PRINT("aclInit failed. ERROR: %d\n", ret); return ret);
    return 0;
}

template <typename T>
int CreateAclTensor(const std::vector<T>& hostData, const std::vector<int64_t>& shape, void** deviceAddr,
                    aclDataType dataType, aclTensor** tensor) {
    auto size = GetShapeSize(shape) * sizeof(T);
    // 调用aclrtMalloc申请device侧内存
    auto ret = aclrtMalloc(deviceAddr, size, ACL_MEM_MALLOC_HUGE_FIRST);
    CHECK_RET(ret == ACL_SUCCESS, LOG_PRINT("aclrtMalloc failed. ERROR: %d\n", ret); return ret);

    // 调用aclrtMemcpy将host侧数据拷贝到device侧内存上
    ret = aclrtMemcpy(*deviceAddr, size, hostData.data(), size, ACL_MEMCPY_HOST_TO_DEVICE);
    CHECK_RET(ret == ACL_SUCCESS, LOG_PRINT("aclrtMemcpy failed. ERROR: %d\n", ret); return ret);

    // 计算连续tensor的strides
    std::vector<int64_t> strides(shape.size(), 1);
    for (int64_t i = shape.size() - 2; i >= 0; i--) {
       strides[i] = shape[i + 1] * strides[i + 1];
    }

    // 调用aclCreateTensor接口创建aclTensor
    *tensor = aclCreateTensor(shape.data(), shape.size(), dataType, strides.data(), 0, aclFormat::ACL_FORMAT_ND,
                              shape.data(), shape.size(), *deviceAddr);
    return 0;
}

// MSPTI
void UserBufferRequest(uint8_t **buffer, size_t *size, size_t *maxNumRecords) {
    LOG_PRINT("========== UserBufferRequest ============\n");
    constexpr uint64_t SIZE = 5 * 1024 * 1024;
    uint8_t *pBuffer = (uint8_t *) malloc(SIZE + ALIGN_SIZE);
    *buffer = ALIGN_BUFFER(pBuffer, ALIGN_SIZE);
    *size = 5 * 1024 * 1024;
    *maxNumRecords = 0;
}

std::atomic<uint64_t> mark_g{0};

// MSPTI
void UserBufferComplete(uint8_t *buffer, size_t size, size_t validSize) {
    LOG_PRINT("========== UserBufferComplete ============\n，%d", validSize );
    if (validSize > 0) {
        msptiActivity *pRecord = NULL;
        msptiResult status = MSPTI_SUCCESS;
        do {
            status = msptiActivityGetNextRecord(buffer, validSize, &pRecord);
            if (status == MSPTI_SUCCESS) {
                if (pRecord->kind == MSPTI_ACTIVITY_KIND_MARKER) {
		            mark_g++;
		            msptiActivityMarker* activity = reinterpret_cast<msptiActivityMarker*>(pRecord);
                               LOG_PRINT("==========pRecord ============ %d, %d\n", activity->sourceKind, activity->flag);
                }
            } else if (status == MSPTI_ERROR_MAX_LIMIT_REACHED) {
                break;
            }
        } while (1);
    }
    free(buffer);
}

aclrtContext context;
aclrtStream stream;

static void test_tx_time()
{
    std::string s(50, 'x');
    aclrtSetCurrentContext(context);
    for (int i = 0; i < 10000; i++) {
        mstxRangeId id = mstxRangeStartA(s.c_str(), stream);
        mstxRangeEnd(id);
    }
}


int main(int argc, const char **argv) {
    int32_t deviceId = 0;
    msptiSubscriberHandle subscriber;
    msptiSubscribe(&subscriber, nullptr, nullptr);
    msptiActivityRegisterCallbacks(UserBufferRequest, UserBufferComplete);
    msptiActivityEnable(MSPTI_ACTIVITY_KIND_MARKER);
    auto ret = Init(deviceId, &context, &stream);
    CHECK_RET(ret == 0, LOG_PRINT("Init acl failed. ERROR: %d\n", ret); return ret);
    std::thread threads[125];
    if (argc != 2) {
        printf("Please Input params with thread_num\n");
        return -1;
    }
    int threadNum = std::stoi(argv[1]);
    int i = 0;

    for (i = 0; i < threadNum; i++)
    {
	    threads[i] = std::thread(test_tx_time); 
    }

    for (i = 0; i < threadNum; i++)
    {
        threads[i].join();
    }

    aclrtDestroyStream(stream);
    aclrtDestroyContext(context);
    aclrtResetDevice(deviceId);
    aclFinalize();
    // MSPTI
    msptiActivityFlushAll(1);
    msptiUnsubscribe(subscriber);
    LOG_PRINT("MSPTI_SMOKE_MARK_NUM %lu\n", mark_g.load());
    return 0;
}

