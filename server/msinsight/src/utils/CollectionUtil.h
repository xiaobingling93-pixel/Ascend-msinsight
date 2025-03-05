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

    // 检查类型T是否支持 == 操作符
    template <typename T>
    struct has_equal_operator : std::integral_constant<bool,
            !std::is_same<decltype(std::declval<T>() == std::declval<T>()), void>::value> {};

    // 模板函数，仅当T支持 == 操作符时才可用
    template <typename T, typename = typename std::enable_if<has_equal_operator<T>::value>::type>
    static inline bool IsVectorEqualIgnoreOrder(std::vector<T> const& vec1, std::vector<T> const& vec2)
    {
        // 如果长度不同，直接返回false
        if (vec1.size() != vec2.size()) {
            return false;
        }

        // 排序两个vector
        std::vector<T> sortedVec1(vec1);
        std::vector<T> sortedVec2(vec2);
        std::sort(sortedVec1.begin(), sortedVec1.end());
        std::sort(sortedVec2.begin(), sortedVec2.end());

        // 比较排序后的vector
        return sortedVec1 == sortedVec2;
    }
};
}
#endif // PROFILER_SERVER_COLLECTIONUTIL_H
