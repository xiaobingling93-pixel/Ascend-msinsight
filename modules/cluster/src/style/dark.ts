/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */
import type { Theme } from '@emotion/react';

export const dark: Theme = {
    contentBackgroundColor: '#1E1E1E', // 区域内容背景色
    closeDragContainerBG: 'rgba(255,255,255,0.11)',
    switchIconColor: '#959595', // 模板下拉按钮颜色
    dividerColor: '#000',
    maskColor: 'rgb(0, 0, 0, 0.55)', // 录制、分析等蒙层颜色
    tooltipBGColor: '#404040',
    tooltipFontColor: 'rgba(255, 255, 255, .9)',
    tooltipBoxShadow: '0 4px 16px 0 rgba( 0, 0, 0, .32 )',
    colorPalette: {
        slateblue: '#6259DE',
        royalblue: '#317AF7',
        skyblue: '#4694C2',
        turquoise: '#5AADA0',
        olivedrab: '#5BA854',
        yellowgreen: '#86AD53',
        gold: '#D1A738',
        orange: '#E08C3A',
        coral: '#DB6B42',
        orangered: '#D94838',
        palevioletred: '#CC5286',
        mediumorchid: '#8C55C2',
        transparentMask: '#4E4E4E',
        pink: '#E67C92',
        otherColor: 'rgb(80, 80, 80)',
        yellow: '#b09239',
    },
};
