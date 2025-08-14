/**
 * Copyright (c) Jupyter Development Team.
 * Distributed under the terms of the BSD-3-Clause License.
 *
 * This file is based on code from the @lumino/coreutils package:
 * https://github.com/jupyterlab/lumino/tree/master/packages/coreutils
 *
 * Modifications made by Huawei Technologies Co., Ltd., 2025.
 */

export type JSONPrimitive = boolean | number | string | null;

export type JSONValue = JSONPrimitive | JSONObject | JSONArray;

export interface JSONObject {
  [key: string]: JSONValue;
}

export interface JSONArray extends Array<JSONValue> {}

export interface ReadonlyPartialJSONObject {
    readonly [key: string]: ReadonlyPartialJSONValue | undefined;
}

export type ReadonlyPartialJSONValue =
  | JSONPrimitive
  | ReadonlyPartialJSONObject
  | ReadonlyPartialJSONArray;

export interface ReadonlyPartialJSONArray
    extends ReadonlyArray<ReadonlyPartialJSONValue> {}

export namespace JSONExt {
    export function isPrimitive(
        value: ReadonlyPartialJSONValue
    ): value is JSONPrimitive {
        return (
            value === null ||
            typeof value === 'number' ||
            typeof value === 'string' ||
            typeof value === 'boolean'
        );
    }

    export function isArray(
        value: ReadonlyPartialJSONValue
    ): value is ReadonlyPartialJSONArray;
    export function isArray(value: ReadonlyPartialJSONValue): boolean {
        return Array.isArray(value);
    }

    export function deepEqual(
        first: ReadonlyPartialJSONValue,
        second: ReadonlyPartialJSONValue
    ): boolean {
        if (first === second) {
            return true;
        }

        if (isPrimitive(first) || isPrimitive(second)) {
            return false;
        }

        let a1 = isArray(first);
        let a2 = isArray(second);

        if (a1 !== a2) {
            return false;
        }

        if (a1 && a2) {
            return deepArrayEqual(
                first as ReadonlyPartialJSONArray,
                second as ReadonlyPartialJSONArray
            );
        }

        return deepObjectEqual(
            first as ReadonlyPartialJSONObject,
            second as ReadonlyPartialJSONObject
        );
    }

    function deepArrayEqual(
        first: ReadonlyPartialJSONArray,
        second: ReadonlyPartialJSONArray
    ): boolean {
        if (first === second) {
            return true;
        }

        if (first.length !== second.length) {
            return false;
        }

        for (let i = 0, n = first.length; i < n; ++i) {
            if (!deepEqual(first[i], second[i])) {
                return false;
            }
        }

        return true;
    }

    function deepObjectEqual(
        first: ReadonlyPartialJSONObject,
        second: ReadonlyPartialJSONObject
    ): boolean {
        if (first === second) {
            return true;
        }

        for (let key in first) {
            if (first[key] !== undefined && !(key in second)) {
                return false;
            }
        }

        for (let key in second) {
            if (second[key] !== undefined && !(key in first)) {
                return false;
            }
        }

        for (let key in first) {
            if (Object.prototype.hasOwnProperty.call(first, key)) {
                let firstValue = first[key];
                let secondValue = second[key];

                if (firstValue === undefined && secondValue === undefined) {
                    continue;
                }

                if (firstValue === undefined || secondValue === undefined) {
                    return false;
                }

                if (!deepEqual(firstValue, secondValue)) {
                    return false;
                }
            }
        }

        return true;
    }
}
