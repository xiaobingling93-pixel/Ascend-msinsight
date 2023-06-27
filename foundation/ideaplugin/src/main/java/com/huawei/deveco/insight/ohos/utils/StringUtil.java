/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2020-2022. All rights reserved.
 */

package com.huawei.deveco.insight.ohos.utils;

/**
 * StringUtil
 *
 * @since 2022-11-24
 */
public class StringUtil {
    /**
     * Anonymous String
     *
     * @param str 待匿名化的字符串
     * @return res 匿名化后的结果
     */
    public static String anonymousString(String str) {
        if (str == null || str.length() < 3) {
            return str;
        }
        int pos = str.length() / 3;
        String anonymousStr = "*".repeat(pos);
        StringBuilder sb = new StringBuilder(str);
        sb.replace(pos, pos * 2, anonymousStr);
        return sb.toString();
    }

    /**
     * 获取占位符字符串
     *
     * @param size 占位符个数
     * @return 占位符字符串
     */
    public static String getPlaceholders(int size) {
        StringBuilder sb = new StringBuilder();
        for (int i = 0; i < size - 1; i++) {
            sb.append("?,");
        }
        if (size > 0) {
            sb.append('?');
        }
        return sb.toString();
    }

    /**
     * 动态替换sql语句 in 多条件查询占位符？
     *
     * @param listSize 多条件查询字符串数组d大小
     * @param originSql 原始sql语句
     * @return String 替换占位符后的sql语句
     */
    public static String prepareStatementWithIn(Integer listSize, String originSql) {
        if (listSize <= 1) {
            return originSql;
        }
        StringBuilder placeholder = new StringBuilder();
        placeholder.append('(');
        for (int i = 0; i < listSize; i++) {
            placeholder.append('?');
            if (i != listSize - 1) {
                placeholder.append(',');
            }
        }
        placeholder.append(')');
        return originSql.replace("(?)", placeholder);
    }

    /**
     * 判断字符串是否为空
     *
     * @param value 要判断的字符串
     * @return 是否为空
     */
    public static boolean isEmpty(String value) {
        return value == null || value.length() == 0;
    }
}
