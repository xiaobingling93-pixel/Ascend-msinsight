/**
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */

#ifndef PROFILER_SERVER_COLLECTIONUTIL_H
#define PROFILER_SERVER_COLLECTIONUTIL_H
#include <vector>
#include <set>

namespace Dic {
class CollectionUtil {
public:
    /**
     * 模板方法，求两个基础类型集合的差集
     *
     * @tparam T 模板类型
     * @param sourceCollection 源集合
     * @param subCollection 减去元素的集合
     * @return
     */
    template <typename T>
    static inline std::vector<T> CalDifferenceVector(const std::vector<T>& sourceCollection,
                                                     const std::vector<T>& subCollection)
    {
        std::set<T> set2(subCollection.begin(), subCollection.end());
        std::vector<T> result;

        // 遍历sourceCollection中的每个元素
        for (const T& element : sourceCollection) {
            // 如果subCollection的集合中不存在当前元素，则添加到结果中
            if (set2.find(element) == set2.end()) {
                result.push_back(element);
            }
        }

        return result;
    }
};
}
#endif // PROFILER_SERVER_COLLECTIONUTIL_H
