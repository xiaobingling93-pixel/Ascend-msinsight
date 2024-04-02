import '@emotion/react';

declare module '@emotion/react' {
    export interface Theme {
        contentBackgroundColor: string; // 区域内容背景色
        closeDragContainerBG: string; // 关闭按钮背景色
        switchIconColor: string; // 模板下拉按钮颜色
        maskColor: string; // 录制、分析等蒙层颜色
        dividerColor: string; // 界面面板分割线颜色
    }
}
