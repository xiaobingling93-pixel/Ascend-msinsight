import type { Theme } from '@emotion/react';
import { ReactComponent as HomePageDarkImage } from '../assets/images/ic_user_experience_rate_filled_dark.svg';

export const dark: Theme = {
    backgroundColor: '#222222', // 主界面背景色
    mainPageIcon: HomePageDarkImage, // 首页图标
    timeInsightIcon: '#86AD53', // time Insight 图标颜色
    allocationInsightIcon: '#9E644F', // allocation Insight 图标颜色
    contentBackgroundColor: '#1E1E1E', // 区域内容背景色
    buttonBackgroundColor: '#4C4C4C', // 模板、按钮背景色
    okBackgroundColor: '#5391FF', // ok按钮背景色
    selectBackgroundColor: '#4c4c4c', // session选中背景色
    cardHeadBackgroundColor: '#2C2C2C', // 卡片表头背景色
    fontColor: 'rgb(255, 255, 255, 0.9)', // 字体颜色
    disabledFontColor: 'rgb(255, 255, 255, 0.15)',
    tableHeadFontColor: 'rgb(255, 255, 255, 0.6)', // 表格表头字体颜色
    subtitleColor: 'rgb(255, 255, 255, 0.4)', // session小字体颜色
    solidLine: 'rgb(255, 255, 255, 0.2)', // border分割线颜色
    buttonFontColor: '#5291FF', // 模板、按钮字体颜色
    svgBackgroundColor: '#A3A3A3', // svg设备、homePage图片颜色
    svgPlayBackgroundColor: '#FFFFFF', // svg录制、暂停、删除等图片颜色
    scrollbarColor: '#606060', // 滚动条颜色
    tableRowSelect: '#4C4C4C', // 卡片表格tr选中色
    devicePullDown: '#EBEBEB', // 设备下拉按钮颜色
    switchIconColor: '#959595', // 模板下拉按钮颜色
    maskColor: 'rgb(0, 0, 0, 0.55)', // 录制、分析等蒙层颜色
    selectedChartColor: 'white', // 泳道选择区域左右两边背景色
    frameRelativeLineColor: 'rgb(255, 255, 255, 0.5)', // frame 连线颜色
    selectJankColor: '#FFDDEB', // 异常帧选中之后的边框颜色
    selectNoJankColor: '#EAFFE9', // 正常帧选中之后的边框颜色
    templateBackgroundColor: 'rgb(255, 255, 255, 0.15)', // 模板图标背景色
    templateSVGFillColor: 'rgba(255, 255, 255, 0.6)', // 模板svg图填充色
    selectedTemplateSVGFillColor: 'rgba(255, 255, 255)', // 选中模板svg图填充色
    selectedChartBackgroundColor: '#45649B', // 选中泳道颜色
    selectedChartBorderColor: '#5391ff',
    deviceProcessBackgroundColor: '#262626',
    deviceProcessContentFontColor: 'rgb(226, 226, 226, 0.9)', // 设备和进程框里字体的颜色
    deviceProcessActiveFontColor: 'rgb(237, 237, 237, 0.9)', // 设备和进程下拉框选中范围的字体颜色
    deviceProcessNotActiveFontColor: 'rgb(150, 150, 150, 0.9)', // 设备和进程下拉框未选中范围的字体颜色
    searchBackgroundColor: 'rgb(255, 255, 255, 0.1)', // 搜索框的背景颜色
    tableBorderColor: 'rgb(255, 255, 255, 0.1)', // border分割线颜色
    backIconBackgroundColor: 'rgb(255, 255, 255)', // all sessions页面返回按钮背景色
    arrowUnexpandedBgColor: 'rgb(255, 255, 255, 0.5)', // Detail表格未展开时小三角背景色
    insightHeaderButtonBackgroundColor: 'rgb(48, 48, 48)',
    timelineAxisColor: 'rgb(76, 76, 76)',
    searchInputCaretColor: '#317AF7', // 搜索输入框光标颜色
    allSessionHeadBgColor: 'rgba(255, 255, 255, 0.05)', // session列表页头部背景色
    deviceProcessActiveBackgroundColor: '#313131', // 设备和进程下拉框选中范围的背景颜色
    placeholderFontColor: 'rgb(100, 100, 100)',
    unitTagInfoBackgroundColor: 'rgb(255, 255, 255, 0.15)', // 泳道tag背景色
    tooltipBGColor: '#404040',
    chartWrongBGColor: 'rgba(217, 72, 56, 0.1)',
    systemEventColor: '#DB6B42', // 系统事件背景色
    thumbIconBackgroundColor: 'rgb(171, 171, 171)', // 推荐入口主图标背景色
    thumbEntranceBackgroundColor: 'rgb(64, 64, 64)', // 推荐入口面板背景颜色
    toolTipBackgroundColor: 'rgb(64, 64, 64)', // 推荐入口面板背景颜色
    toolTipShadowColor: 'rgb(0 0 0 / 10%)',
    shadowBackgroundColor: 'rgb(18, 18, 18)',
    timeMakerListToolTipBackgroundColor: '#1A1A1A',
    enclosureBorder: 'rgb(131, 131, 131)', // 封闭框线颜色，如：Input 组件、Radio 按钮
    controllerBarBackgroundColor: 'rgb(66, 66, 66)', // Detail区域上方控制栏背景色
    activeButtonBackgroundColor: 'rgb(255, 255, 255)', // 活动按钮背景色
    disableButtonBackgroundColor: 'rgb(127, 127, 127)', // 失效按钮背景色
    fpsPointersColor: 'rgb(255, 255, 255, 0.7)', // FPS指针
    fpsTextColor: 'rgb(255, 255, 255)', // FPS文字颜色
    fpsColor: 'rgb(255, 255, 255, 0.5)', // FPS本身颜色
    categories: {
        System: { name: 'System', color: '#5291ff', background: '#3a404d' }, // 系统分类标签颜色与背景色
        ArkTS: { name: 'ArkTS', color: '#5ba854', background: '#3b423a' }, // JS分类标签颜色与背景色
        NAPI: { name: 'NAPI', color: '#db6b42', background: '#5b504c' }, // NAPI分类标签颜色与背景色
        Native: { name: 'Native', color: '#db6b42', background: '#5b504c' }, // Native分类标签颜色与背景色
    },
    searchIconBackgroundColor: 'rgb(255, 255, 255)', // 全部session页搜索按钮背景色
    cancelIconBackgroundColor: 'rgb(255, 255, 255, 0.6)', // 全部session页删除按钮背景色
    dividerColor: '#000',
    closeDragContainerBG: 'rgba(255,255,255,0.11)',
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
    schedulingBorderColorPalette: {
        slateblue: '#F0EFFF',
        royalblue: '#EAF2FF',
        skyblue: '#E8F6FF',
        turquoise: '#E8FFFB',
        olivedrab: '#EAFFE9',
        yellowgreen: '#B6D8FF',
        gold: '#FFF8E7',
        orange: '#FFF2E6',
        coral: '#FFE8DF',
        orangered: '#FFE4E1',
        palevioletred: '#FFDDEB',
        mediumorchid: '#F3E3FF',
        pink: '#FFDDEB',
        otherColor: 'rgb(80, 80, 80)',
        yellow: '#b09239',
    },
    grayscaleColor: '#4E4E4E',
    filterColor: 'rgba(255, 255, 255, .9)', // 过滤模块字体颜色
    filterTipColor: 'rgba(255, 255, 255, .6)', // 过滤模块提示文字字体颜色
    multiSelectBgColor: 'rgba(255, 255, 255, .15)', // 多选列表item背景色
    multiSelectUnCheckedBgColor: 'rgba(0, 0, 0, .4)', // 多选列表item单选框未选中背景色
    multiSelectUnCheckedBorderColor: 'rgba(255, 255, 255, .4)', // 多选列表item单选框未选中边框色
    timeDiffBackgroundColor: 'rgba(33, 33, 33, 0.82)', // 时间差背景颜色
    timeDiffPictureColor: 'rgb(150, 150, 150, 0.9)', // 时间差箭头颜色
    filterIconColor: 'rgb(255, 255, 255)', // 过滤图标背景色
    filterSelectActiveBgColor: '#313131',
    searchContainerBorder: 'none',
    searchHeaderBgColor: 'rgb(255, 255, 255, 0.1)',
    listItemHoverColor: 'rgb(255, 255, 255, 0.7)',
    flagListSelectedColor: 'rgb(91,94,103)',
    flagListHoverColor: 'rgb(79, 79, 79)',
    colorSelectedBorder: '1px solid #FFFFFF',
    frameExpectColor: 'rgb(255, 255, 255, 0.9)',
    schedulingLineColor: 'rgb(255, 255, 255, 0.7)',
    buttonColor: {
        enableClickColor: 'rgba(255,255,255,0.6)',
        disableClickColor: 'rgba(255,255,255,0.2)',
        emphasizeColor: '#5291FF',
        suspendBGColor: 'rgba(255,255,255,0.1)',
        unSuspendBGColor: 'transparent',
    },
    otherColor: 'grey',
    switchOpen: '#5291FF',
    switchClose: '#1A1A1A',
    // 把新加的字段同时加到light.ts文件、以及emotion.d.ts文件...........
};
