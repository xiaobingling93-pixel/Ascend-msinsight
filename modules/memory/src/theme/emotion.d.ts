import '@emotion/react';
import React from 'react';

declare module '@emotion/react' {
    export interface Theme {
        tooltipBGColor: string; // tooltip背景颜色
        tooltipFontColor: string; // ToolTip字体颜色
        tooltipBoxShadow: string; // ToolTip盒子阴影
    }
}
