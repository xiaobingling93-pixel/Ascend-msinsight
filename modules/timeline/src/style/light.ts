import type { Theme } from '@emotion/react';
import { ReactComponent as HomePageLightImage } from '../assets/images/ic_user_experience_rate_filled_light.svg';

export const light: Theme = {
    backgroundColor: '#FFFFFF',
    mainPageIcon: HomePageLightImage,
    timeInsightIcon: '#A5D61D', // time Insight 图标颜色
    allocationInsightIcon: '#ED955F', // allocation Insight 图标颜色
    contentBackgroundColor: '#F1F3F5',
    buttonBackgroundColor: '#E5E6E8',
    okBackgroundColor: '#0A58F6',
    selectBackgroundColor: '#FFFFFF',
    cardHeadBackgroundColor: '#FFFFFF',
    fontColor: 'rgba(0, 0, 0, 0.9)',
    disabledFontColor: 'rgb(171, 171, 171)',
    tableHeadFontColor: 'rgb(0, 0, 0, 0.6)',
    subtitleColor: 'rgb(0, 0, 0, 0.5)',
    solidLine: 'rgb(0, 0, 0, 0.2)',
    buttonFontColor: '#0A59F7',
    svgBackgroundColor: 'rgb(0,0,0,0.9)',
    svgPlayBackgroundColor: '#000000',
    scrollbarColor: '#C1C2C4',
    tableRowSelect: '#D9DADC',
    devicePullDown: '#1A1819',
    switchIconColor: '#18181A',
    maskColor: 'rgb(255, 255, 255, 0.55)',
    templateBackgroundColor: 'rgb(255, 255, 255)',
    templateSVGFillColor: 'rgba(0, 0, 0, 0.6)',
    selectedTemplateSVGFillColor: 'rgba(255, 255, 255)',
    selectedChartColor: 'black',
    frameRelativeLineColor: 'rgb(0, 0, 0, 0.5)', // frame 连线颜色
    selectJankColor: '#C0250D', // 异常帧选中之后的边框颜色
    selectNoJankColor: '#25A707', // 正常帧选中之后的边框颜色
    selectedChartBackgroundColor: '#ACC3F5',
    selectedChartBorderColor: '#0959f4',
    deviceProcessBackgroundColor: '#FFFFFF',
    deviceProcessContentFontColor: 'rgb(0, 0, 0, 0.9)', // 设备和进程框里字体的颜色
    deviceProcessActiveFontColor: 'rgb(0, 0, 0, 0.9)', // 设备和进程下拉框选中范围的字体颜色
    deviceProcessNotActiveFontColor: 'rgb(150, 150, 150, 0.9)', // 设备和进程下拉框未选中范围的字体颜色
    searchBackgroundColor: 'rgb(0, 0, 0, 0.1)',
    tableBorderColor: 'rgb(0, 0, 0, 0.1)',
    backIconBackgroundColor: 'rgb(0, 0, 0)',
    arrowUnexpandedBgColor: 'rgb(0, 0, 0, 0.5)',
    insightHeaderButtonBackgroundColor: 'rgb(217, 218, 220)',
    timelineAxisColor: 'rgb(217, 217, 220)',
    searchInputCaretColor: '#0A59F7',
    allSessionHeadBgColor: 'rgba(0, 0, 0, 0.05)',
    deviceProcessActiveBackgroundColor: '#FFFFFF', // 设备和进程下拉框选中范围的背景颜色
    placeholderFontColor: 'rgb(150, 150, 150, 0.9)',
    unitTagInfoBackgroundColor: 'rgb(0, 0, 0, 0.15)',
    tooltipBGColor: 'rgb(255, 255, 255)',
    chartWrongBGColor: 'rgba(232, 64, 38, 0.1)',
    systemEventColor: '#ED6F21', // 系统事件背景色
    thumbIconBackgroundColor: 'rgb(51, 51, 51)',
    thumbEntranceBackgroundColor: 'rgb(64, 64, 64)',
    toolTipBackgroundColor: 'rgb(241, 243, 245)',
    toolTipShadowColor: 'rgba(200, 200, 200, 0.2)',
    timeMakerListToolTipBackgroundColor: '#404040',
    shadowBackgroundColor: 'rgb(121, 121, 121)',
    enclosureBorder: 'rgb(201, 201, 201)', // 封闭框线颜色，如：Input 组件、Radio 按钮
    controllerBarBackgroundColor: 'rgb(235, 235, 235)', // Detail区域上方控制栏背景色
    activeButtonBackgroundColor: 'rgb(0, 0, 0)',
    disableButtonBackgroundColor: 'rgb(127, 127, 127)',
    fpsPointersColor: 'rgb(0, 0 0, 0.7)', // FPS指针
    fpsTextColor: 'rgb(0, 0, 0)', // FPS文字颜色
    fpsColor: 'rgb(0, 0, 0, 0.5)', // FPS本身颜色
    categories: {
        System: { name: 'System', color: '#0A59F7', background: '#E7EFFE' }, // 系统分类标签颜色与背景色
        ArkTS: { name: 'ArkTS', color: '#64BB5C', background: '#EFF6EE' }, // JS分类标签颜色与背景色
        NAPI: { name: 'NAPI', color: '#ED6F21', background: '#FBF0EC' }, // NAPI分类标签颜色与背景色
        Native: { name: 'Native', color: '#ED6F21', background: '#FBF0EC' }, // Native分类标签颜色与背景色
    },
    searchIconBackgroundColor: 'rgb(0, 0, 0)',
    cancelIconBackgroundColor: 'rgb(0, 0, 0, 0.6)',
    dividerColor: '#fff',
    closeDragContainerBG: 'rgb(229, 230, 232)',
    tooltipFontColor: 'rgba(0, 0, 0, .6)',
    tooltipBoxShadow: '0 4px 16px 0 rgba( 0, 0, 0, .16 )',
    colorPalette: {
        slateblue: '#564AF7',
        royalblue: '#0A59F7',
        skyblue: '#46B1E3',
        turquoise: '#61CFBE',
        olivedrab: '#64BB5C',
        yellowgreen: '#A5D61D',
        gold: '#F7CE00',
        orange: '#F9A01E',
        coral: '#ED6F21',
        orangered: '#E84026',
        palevioletred: '#E64566',
        mediumorchid: '#AC49F5',
        transparentMask: '#CCCCCC',
        pink: '#E67C92',
        otherColor: 'rgb(220, 220, 220)',
        yellow: '#b09239',
    },
    schedulingBorderColorPalette: {
        slateblue: '#3C31C8',
        royalblue: '#0B44B7',
        skyblue: '#0A7FB6',
        turquoise: '#07AE94',
        olivedrab: '#25A707',
        yellowgreen: '#76A100',
        gold: '#C1A208',
        orange: '#C17300',
        coral: '#BE4900',
        orangered: '#B81900',
        palevioletred: '#B70025',
        mediumorchid: '#8500C8',
        pink: '#C0250D',
        otherColor: 'rgb(220, 220, 220)',
        yellow: '#b09239',
    },
    grayscaleColor: '#CCCCCC',
    filterColor: 'rgba(0, 0, 0, .9)', // 过滤模块字体颜色
    filterTipColor: 'rgba(0, 0, 0, .6)', // 过滤模块提示文字字体颜色
    multiSelectBgColor: 'rgba(0, 0, 0, .15)', // 多选列表item背景色
    multiSelectUnCheckedBgColor: 'rgba(255, 255, 255, .4)', // 多选列表item单选框未选中背景色
    multiSelectUnCheckedBorderColor: 'rgba(0, 0, 0, .4)', // 多选列表item单选框未选中边框色
    timeDiffBackgroundColor: 'rgb(150, 150, 150, 0.9)', // 时间差背景颜色
    timeDiffPictureColor: 'rgba(0, 0, 0, 0.82)', // 时间差箭头颜色
    filterIconColor: 'rgb(0, 0, 0, .6)', // 过滤图标背景色
    filterSelectActiveBgColor: 'rgb(0, 0, 0, .1)',
    searchContainerBorder: '1px solid rgb(0, 0, 0, .3)',
    searchHeaderBgColor: 'transparent',
    listItemHoverColor: 'rgb(0, 0, 0, 0.7)',
    flagListSelectedColor: 'rgb(164,161,152)',
    flagListHoverColor: 'rgb(176, 176, 176)',
    colorSelectedBorder: '1px solid #000000',
    schedulingLineColor: 'black',
    frameExpectColor: 'rgb(0, 0, 0, 0.9)',
    buttonColor: {
        enableClickColor: 'rgba(0,0,0,0.6)',
        disableClickColor: 'rgba(0,0,0,0.2)',
        emphasizeColor: '#5291FF',
        suspendBGColor: 'rgba(0,0,0,0.1)',
        unSuspendBGColor: 'transparent',
    },
    otherColor: 'rgb(192,192,192)',
    switchOpen: '#0A59F7',
    switchClose: '#E1E3E6',
};
