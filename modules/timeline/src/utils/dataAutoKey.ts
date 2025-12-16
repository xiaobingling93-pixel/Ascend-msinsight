/*
 * -------------------------------------------------------------------------
 * This file is part of the MindStudio project.
 * Copyright (c) 2025 Huawei Technologies Co.,Ltd.
 *
 * MindStudio is licensed under Mulan PSL v2.
 * You can use this software according to the terms and conditions of the Mulan PSL v2.
 * You may obtain a copy of Mulan PSL v2 at:
 *
 *          http://license.coscl.org.cn/MulanPSL2
 *
 * THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
 * EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
 * MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
 * See the Mulan PSL v2 for more details.
 * -------------------------------------------------------------------------
 */
const autoKey = Symbol('autokey');

export type AutoKey<T extends object> = T & { [autoKey]?: string };

export const getAutoKey = (function<T extends object>(): (data: AutoKey<T>) => string {
    let counter = 0;
    const generateRowKey = (): string => {
        if (counter === Number.MAX_SAFE_INTEGER) {
            counter = 0;
        }
        return `${counter++}`;
    };
    return (data: AutoKey<T>): string => {
        let key = data[autoKey];
        if (key === undefined) {
            key = data[autoKey] = generateRowKey();
        }
        return key;
    };
})();
