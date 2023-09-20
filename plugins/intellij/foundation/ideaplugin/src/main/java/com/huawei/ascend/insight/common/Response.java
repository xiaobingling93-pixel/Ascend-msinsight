/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2020-2022. All rights reserved.
 */

package com.huawei.ascend.insight.common;

import com.huawei.ascend.insight.common.constant.Constant;
import com.huawei.ascend.insight.utils.LogPrinter;

import com.google.gson.annotations.SerializedName;

import lombok.Getter;

import org.jetbrains.annotations.NotNull;

import java.lang.reflect.Field;
import java.lang.reflect.InvocationTargetException;
import java.util.HashMap;
import java.util.Map;

/**
 * Unified Response
 *
 * @param <T> generic response type
 * @since 2023-02-08
 */
@Getter
public class Response<T> {
    private static final LogPrinter LOGGER = LogPrinter.createLogger(Response.class);

    /**
     * Response status, serialize to `result`
     */
    @SerializedName("result")
    private Boolean isSuccess;

    /**
     * Generic return body
     */
    private final T body;

    /**
     * Insight error, including error code and message
     */
    @SerializedName("error")
    private InsightError insightError;

    private Response(Boolean isSuccess) {
        this(isSuccess, null, null);
    }

    private Response(Boolean isSuccess, InsightError insightError) {
        this(isSuccess, null, insightError);
    }

    private Response(Boolean isSuccess, T body) {
        this(isSuccess, body, null);
    }

    private Response(Boolean isSuccess, T body, InsightError insightError) {
        this.isSuccess = isSuccess;
        this.body = body;
        this.insightError = insightError;
    }

    /**
     * Success response builder
     *
     * @return Success Response
     */
    public static Response<?> success() {
        return new Response<>(Constant.IS_SUCCESS);
    }

    /**
     * Success response builder
     *
     * @param responseBody response DTO
     * @return Success Response
     */
    public static <R> Response<R> success(R responseBody) {
        return new Response<>(Constant.IS_SUCCESS, responseBody);
    }

    /**
     * Failure with Insight error code and error message
     *
     * @param insightError Insight error code & message
     * @return Failure Response
     */
    public static Response<?> failure(@NotNull InsightError insightError) {
        return new Response<>(Constant.IS_FAILURE, insightError);
    }

    /**
     * Merge two responses of type Response<T> to a unique response
     * Caution: when calling this function, you must provide a non-parameter constructor of class R
     * and you must ensure that response to be merged contains the same name and same type fields
     * with the provided response
     *
     * @param lhs left hand side response
     * @param rhs right hand side response
     * @param responseType Response of return class type
     * @return response of type R
     */
    public static <T, V, R> Response<R> mergeResponse(Response<T> lhs, Response<V> rhs, Class<R> responseType) {
        if (lhs == null || rhs == null || lhs.getBody() == null || rhs.getBody() == null) {
            LOGGER.warn("Response to be merged is null");
            return new Response<>(Constant.IS_FAILURE);
        }
        // no error msg yet, no need to merge
        if (Boolean.FALSE.equals(lhs.getIsSuccess()) || Boolean.FALSE.equals(rhs.getIsSuccess())) {
            LOGGER.warn("Response status is failure, no need to merge");
            return new Response<>(Constant.IS_FAILURE);
        }
        Class<?> lhsClazz = lhs.getBody().getClass();
        Class<?> rhsClazz = rhs.getBody().getClass();
        Map<String, Field> lhsFieldMap = new HashMap<>();
        Map<String, Field> rhsFieldMap = new HashMap<>();
        for (Field field : lhsClazz.getDeclaredFields()) {
            lhsFieldMap.put(field.getName(), field);
        }
        for (Field field : rhsClazz.getDeclaredFields()) {
            rhsFieldMap.put(field.getName(), field);
        }
        R response;
        try {
            response = responseType.getConstructor().newInstance();
            // every fields that response class contains
            for (Field field : responseType.getDeclaredFields()) {
                checkAndSetSameField(field, response, lhs, lhsFieldMap, responseType);
                checkAndSetSameField(field, response, rhs, rhsFieldMap, responseType);
            }
        } catch (InstantiationException | IllegalAccessException | InvocationTargetException | NoSuchFieldException
            | NoSuchMethodException e) {
            LOGGER.warn("Failed to merge two responses, cause: ", e.getMessage());
            return new Response<>(Constant.IS_FAILURE);
        }
        return new Response<>(Constant.IS_SUCCESS, response);
    }

    private static <R, T> void checkAndSetSameField(Field field, R mergedResponse, Response<T> providedResponse,
                                                    Map<String, Field> fieldMap, Class<R> responseType)
            throws NoSuchFieldException, IllegalAccessException {
        String fieldName = field.getName();
        if (fieldMap.containsKey(fieldName)
                && field.getGenericType().equals(fieldMap.get(fieldName).getGenericType())) {
            Field mergedField = fieldMap.get(fieldName);
            Field responseField = responseType.getDeclaredField(fieldName);
            mergedField.setAccessible(true);
            responseField.setAccessible(true);
            responseField.set(mergedResponse, mergedField.get(providedResponse.getBody()));
        }
    }
}
