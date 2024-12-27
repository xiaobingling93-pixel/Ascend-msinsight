/**
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */

#ifndef PROFILER_SERVER_COLLECTIONUTIL_H
#define PROFILER_SERVER_COLLECTIONUTIL_H
#include <vector>
#include <set>
#include <unordered_set>

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

    template <typename T>
    static inline std::vector<T> CalIntersection(const std::vector<T>& vec1, const std::vector<T>& vec2)
    {
        std::unordered_set<T> set1(vec1.begin(), vec1.end());
        std::vector<T> result;

        for (const auto &item: vec2) {
            if (set1.count(item) > 0) {
                result.push_back(item);
                set1.erase(item);
            }
        }
        return result;
    }
};
}
#endif // PROFILER_SERVER_COLLECTIONUTIL_H
