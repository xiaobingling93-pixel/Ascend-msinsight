/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2024. All rights reserved.
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
