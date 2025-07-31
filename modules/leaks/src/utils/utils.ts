/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2025-2025. All rights reserved.
 */
export const chartResize = (ins: echarts.ECharts | null | undefined): void => {
    // ehcarts的custom在渲染超大量数据时会出现残影残留，通过手动触发resize可以消除残影的影响
    if (!ins) {
        return;
    }
    const width = ins.getWidth();
    ins.resize({
        width: width + 1,
    });
    ins.resize({
        width: 'auto',
    });
};
