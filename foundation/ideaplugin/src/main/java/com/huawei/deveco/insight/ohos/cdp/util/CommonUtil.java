/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022-2022. All rights reserved.
 */

package com.huawei.deveco.insight.ohos.cdp.util;

import java.util.ArrayList;
import java.util.List;

/**
 * CommonUtil
 *
 * @since 2022/09/05
 */
public class CommonUtil {
    /**
     * Batch processing of collections
     *
     * @param sourceList sourceList
     * @param maxBatchSize int
     * @return List<List<T>> batchList
     */
    public static <T> List<List<T>> splitList(List<T> sourceList, int maxBatchSize) {
        List<List<T>> batchList = new ArrayList<>();
        int size = sourceList.size();
        int remainder = size % maxBatchSize;
        int count = (size / maxBatchSize);
        for (int i = 0; i < count; i++) {
            batchList.add(sourceList.subList(i * maxBatchSize, (i + 1) * maxBatchSize));
        }
        if (remainder > 0) {
            batchList.add(sourceList.subList(count * maxBatchSize, count * maxBatchSize + remainder));
        }
        return batchList;
    }


    /**
     * castList
     *
     * @param obj obj
     * @param clazz clazz
     * @param <T> <T>
     * @return List<T>
     */
    public static <T> List<T> castList(Object obj, Class<T> clazz) {
        List<T> result = new ArrayList<T>();
        if (obj instanceof List<?>) {
            for (Object object : (List<?>) obj) {
                result.add(clazz.cast(object));
            }
            return result;
        }
        return new ArrayList<>();
    }
}
