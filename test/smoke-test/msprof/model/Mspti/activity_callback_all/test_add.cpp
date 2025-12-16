#include <iostream>
#include <vector>
#include "acl/acl.h"
#include "aclnnop/aclnn_add.h"

/**************************** Mspti **************************/
#include "mspti/mspti.h"
#include "mstx/ms_tools_ext.h"
/**************************** Mspti **************************/

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

void UserBufferRequest(uint8_t **buffer, size_t *size, size_t *maxNumRecords) {
	constexpr uint32_t SIZE = 5 * 1024 * 1024;
	uint8_t *pBuffer = (uint8_t*)malloc(SIZE);
	*buffer = pBuffer;
	*size = SIZE;
	*maxNumRecords = 0;
}

static void ShowApiInfo(msptiActivityApi* api) {
	if (api == nullptr) {
		printf("Api nullptr\n");
		return;
	}
	printf("[Api]kind: %d, start: %lu, end: %lu, correlationId: %lu, name: %s\n",
			api->kind, api->start, api->end, api->correlationId, api->name);
}

static void ShowKernelInfo(msptiActivityKernel* kernel) {
	if (kernel == nullptr) {
		printf("Kernel nullptr\n");
		return;
	}
	printf("[Kernel]kind: %d, start: %lu, end: %lu, correlatonId: %lu, type: %s, name: %s\n",
			kernel->kind, kernel->start, kernel->end, kernel->correlationId, kernel->type, kernel->name);
}

static void ShowMarkerInfo(msptiActivityMarker* marker) {
	if (marker == nullptr) {
		printf("Marker nullptr\n");
		return;
	}
	printf("[Marker]kind: %d, flag: %d, sourceKind: %d, timestamp: %lu, id: %lu, name: %s\n",
			marker->kind, marker->flag, marker->sourceKind, marker->timestamp, marker->id, marker->name);
}

void UserBufferComplete(uint8_t *buffer, size_t size, size_t validSize) {
	if (validSize > 0) {
		msptiActivity *pRecord = nullptr;
		msptiResult status = MSPTI_SUCCESS;
		do {
			status = msptiActivityGetNextRecord(buffer, validSize, &pRecord);
			if (status == MSPTI_SUCCESS) {
				if (pRecord->kind == MSPTI_ACTIVITY_KIND_API) {
					msptiActivityApi* api = reinterpret_cast<msptiActivityApi*>(pRecord);
					ShowApiInfo(api);
				} else if (pRecord->kind == MSPTI_ACTIVITY_KIND_KERNEL) {
					msptiActivityKernel* kernel = reinterpret_cast<msptiActivityKernel*>(pRecord);
					ShowKernelInfo(kernel);
				} else if (pRecord->kind == MSPTI_ACTIVITY_KIND_MARKER) {
					msptiActivityMarker* marker = reinterpret_cast<msptiActivityMarker*>(pRecord);
					ShowMarkerInfo(marker);
				}
			} else if (status == MSPTI_ERROR_MAX_LIMIT_REACHED) {
				break;
			}
		} while (1);
	}
}

void UserCallback(void *pUserData, msptiCallbackDomain domain, msptiCallbackId callbackId, const msptiCallbackData *pCallbackInfo) {
	if (pCallbackInfo->callbackSite == MSPTI_API_ENTER) {
		mstxMarkA("Start", nullptr);
	} else if (pCallbackInfo->callbackSite == MSPTI_API_EXIT) {
		mstxMarkA("End", nullptr);
	}
}

int main() {
	// 1.(固定写法)device/context/stream初始化
	// 根据自己的实际device填写deviceId
	int32_t deviceId = 0;
	aclrtContext context;
	aclrtStream stream;
	/* ****************************************** Mspti **************************************/
	msptiSubscriberHandle subscriber{nullptr};
	msptiSubscribe(&subscriber, UserCallback, nullptr);
	msptiEnableCallback(1, subscriber, MSPTI_CB_DOMAIN_RUNTIME, MSPTI_CBID_RUNTIME_CONTEXT_CREATED_EX);
	msptiEnableCallback(1, subscriber, MSPTI_CB_DOMAIN_RUNTIME, MSPTI_CBID_RUNTIME_MALLOC);
        msptiEnableCallback(1, subscriber, MSPTI_CB_DOMAIN_RUNTIME, MSPTI_CBID_RUNTIME_FREE);
	msptiActivityRegisterCallbacks(UserBufferRequest, UserBufferComplete);
	msptiActivityEnable(MSPTI_ACTIVITY_KIND_MARKER);
        msptiActivityEnable(MSPTI_ACTIVITY_KIND_KERNEL);
	msptiActivityEnable(MSPTI_ACTIVITY_KIND_API);
	/* ****************************************** Mspti **************************************/

	auto ret = Init(deviceId, &context, &stream);
	// check根据自己的需要处理
	CHECK_RET(ret == 0, LOG_PRINT("Init acl failed. ERROR: %d\n", ret); return ret);
	// 2.构造输入与输出，需要根据API的接口自定义构造
	std::vector<int64_t> selfShape = {4, 2};
	std::vector<int64_t> otherShape = {4, 2};
	std::vector<int64_t> outShape = {4, 2};
	void* selfDeviceAddr = nullptr;
	void* otherDeviceAddr = nullptr;
	void* outDeviceAddr = nullptr;
	aclTensor* self = nullptr;
	aclTensor* other = nullptr;
	aclScalar* alpha = nullptr;
	aclTensor* out = nullptr;
	std::vector<float> selfHostData = {0, 1, 2, 3, 4, 5, 6, 7};
	std::vector<float> otherHostData = {1, 1, 1, 2, 2, 2, 3, 3};
	std::vector<float> outHostData = {0, 0, 0, 0, 0, 0, 0, 0};
	float alphaValue = 1.2f;
	// 创建self aclTensor
	ret = CreateAclTensor(selfHostData, selfShape, &selfDeviceAddr, aclDataType::ACL_FLOAT, &self);
	CHECK_RET(ret == ACL_SUCCESS, return ret);
	// 创建other aclTensor
	ret = CreateAclTensor(otherHostData, otherShape, &otherDeviceAddr, aclDataType::ACL_FLOAT, &other);
	CHECK_RET(ret == ACL_SUCCESS, return ret);
	// 创建alpha aclScalar
	alpha = aclCreateScalar(&alphaValue, aclDataType::ACL_FLOAT);
	CHECK_RET(alpha != nullptr, return ret);
	// 创建out aclTensor
	ret = CreateAclTensor(outHostData, outShape, &outDeviceAddr, aclDataType::ACL_FLOAT, &out);
	CHECK_RET(ret == ACL_SUCCESS, return ret);

	// 3.调用CANN算子库API
	uint64_t workspaceSize = 0;
	aclOpExecutor* executor;
	// 调用aclnnAdd第一段接口
	ret = aclnnAddGetWorkspaceSize(self, other, alpha, out, &workspaceSize, &executor);
	CHECK_RET(ret == ACL_SUCCESS, LOG_PRINT("aclnnAddGetWorkspaceSize failed. ERROR: %d\n", ret); return ret);
	// 根据第一段接口计算出的workspaceSize申请device内存
	void* workspaceAddr = nullptr;
	if (workspaceSize > 0) {
		ret = aclrtMalloc(&workspaceAddr, workspaceSize, ACL_MEM_MALLOC_HUGE_FIRST);
		CHECK_RET(ret == ACL_SUCCESS, LOG_PRINT("allocate workspace failed. ERROR: %d\n", ret); return ret;);
	}
	// 调用aclnnAdd第二段接口
	ret = aclnnAdd(workspaceAddr, workspaceSize, executor, stream);
	CHECK_RET(ret == ACL_SUCCESS, LOG_PRINT("aclnnAdd failed. ERROR: %d\n", ret); return ret);
	// 4.(固定写法)同步等待任务执行结束
	ret = aclrtSynchronizeStream(stream);
	CHECK_RET(ret == ACL_SUCCESS, LOG_PRINT("aclrtSynchronizeStream failed. ERROR: %d\n", ret); return ret);
	// 5.获取输出的值，将device侧内存上的结果拷贝至host侧，需要根据具体API的接口定义修改
	auto size = GetShapeSize(outShape);
	std::vector<float> resultData(size, 0);
	ret = aclrtMemcpy(resultData.data(), resultData.size() * sizeof(resultData[0]), outDeviceAddr, size * sizeof(float),
			ACL_MEMCPY_DEVICE_TO_HOST);
	CHECK_RET(ret == ACL_SUCCESS, LOG_PRINT("copy result from device to host failed. ERROR: %d\n", ret); return ret);
	//for (int64_t i = 0; i < size; i++) {
	//	LOG_PRINT("result[%ld] is: %f\n", i, resultData[i]);
	//}

	// 6.释放aclTensor和aclScalar，需要根据具体API的接口定义修改
	aclDestroyTensor(self);
	aclDestroyTensor(other);
	aclDestroyScalar(alpha);
	aclDestroyTensor(out);

	// 7.释放device资源，需要根据具体API的接口定义修改
	aclrtFree(selfDeviceAddr);
	aclrtFree(otherDeviceAddr);
	aclrtFree(outDeviceAddr);
	if (workspaceSize > 0) {
		aclrtFree(workspaceAddr);
	}
	aclrtDestroyStream(stream);
	aclrtDestroyContext(context);
	aclrtResetDevice(deviceId);
	aclFinalize();
	/********************************************** Mspti ****************************************/
	msptiEnableCallback(0, subscriber, MSPTI_CB_DOMAIN_RUNTIME, MSPTI_CBID_RUNTIME_CONTEXT_CREATED_EX);
	msptiEnableCallback(0, subscriber, MSPTI_CB_DOMAIN_RUNTIME, MSPTI_CBID_RUNTIME_MALLOC);
    msptiEnableCallback(0, subscriber, MSPTI_CB_DOMAIN_RUNTIME, MSPTI_CBID_RUNTIME_FREE);
    msptiActivityDisable(MSPTI_ACTIVITY_KIND_MARKER);
    msptiActivityDisable(MSPTI_ACTIVITY_KIND_KERNEL);
    msptiActivityDisable(MSPTI_ACTIVITY_KIND_API);
	msptiActivityFlushAll(1);
	msptiUnsubscribe(subscriber);
	/********************************************** Mspti ****************************************/
	return 0;
}

