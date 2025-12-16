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
export class FormatDate {
    formatTimeData = (): string => {
        const dates = new Date();
        const year = dates.getFullYear().toString();
        const months = (dates.getMonth() + 1).toString().padStart(2, '0');
        const day = dates.getDate().toString().padStart(2, '0');
        const hours = dates.getHours().toString().padStart(2, '0');
        const minutes = dates.getMinutes().toString().padStart(2, '0');
        const seconds = dates.getSeconds().toString().padStart(2, '0');
        return `${year}-${months}-${day} ${hours}:${minutes}:${seconds}`;
    };
}
