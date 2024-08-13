/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */
import * as echarts from 'echarts';
import type { EChartsType } from 'echarts';
declare global {
    interface Window {
        // for inspecting some internal state
        _listenerMap: {[props: string]: any};
    }
};

class DomVisibilityListener {
    private _visible: boolean = false;
    private readonly _target: HTMLElement | null;

    private _listener: any;

    private readonly _onVisibleChange?: (visible: boolean) => void;
    constructor(dom: string, onVisibleChange?: (visible: boolean) => void) {
        if (window._listenerMap === undefined || window._listenerMap === null) {
            window._listenerMap = {};
        }
        const listenerMap = window._listenerMap;
        if (listenerMap[dom] !== undefined && listenerMap[dom] !== null) {
            listenerMap[dom].clear();
            listenerMap[dom] = null;
        }
        listenerMap[dom] = this;
        this._target = document.getElementById(dom);
        this.visible = this._target?.offsetParent !== null;
        this._onVisibleChange = onVisibleChange;
        this.add();
    }

    get visible(): boolean {
        return this._visible;
    }

    set visible(value: boolean) {
        this._visible = value;
    }

    add(): void {
        this._listener = setTimeout(() => {
            const newStatus = this._target?.offsetParent !== null;
            if (newStatus !== this.visible && this._onVisibleChange !== undefined) {
                this._onVisibleChange(newStatus);
            }
            this.visible = newStatus;
            this.add();
        }, 100);
    }

    clear(): void {
        if (this._listener !== null) {
            clearTimeout(this._listener);
        }
    }
}

export const chartVisbilityListener = (dom: string, onVisibleChange?: (visible?: boolean) => void): DomVisibilityListener => {
    return new DomVisibilityListener(dom, (status: boolean) => {
        if (status && onVisibleChange !== undefined) {
            onVisibleChange();
        }
    });
};

export const checkDomDisplay = (dom: HTMLElement): boolean => {
    return dom?.offsetParent !== null;
};
interface EChartsTypeExtends extends EChartsType {
    addResizeEvent?: boolean;
    addLoadEvent?: boolean;
    resizeFunc?: () => void;
    loadFunc?: () => void;
}
export function getResizeEcharts(chartDom: HTMLElement, myChart: EChartsTypeExtends): EChartsType {
    removeResizeEvent(myChart);
    const newChart = echarts.getInstanceByDom(chartDom)
        ? echarts.getInstanceByDom(chartDom) as echarts.ECharts
        : echarts.init(chartDom);
    addResizeEvent(newChart);
    return newChart;
}
export function addResizeEvent(echart?: EChartsTypeExtends): void {
    if (echart === undefined || echart === null) {
        return;
    }
    if (!echart.addResizeEvent) {
        const resizeFunc = (): void => {
            if (checkDomDisplay(echart.getDom())) {
                echart.resize();
            }
        };
        window.addEventListener('resize', resizeFunc);
        echart.addResizeEvent = true;
        echart.resizeFunc = resizeFunc;
    }
    if (!echart.addLoadEvent) {
        const loadFunc = (): void => {
            if (checkDomDisplay(echart.getDom())) {
                echart.resize();
            }
        };
        window.addEventListener('load', loadFunc);
        echart.addLoadEvent = true;
        echart.loadFunc = loadFunc;
    }
}
export function removeResizeEvent(echart?: EChartsTypeExtends): void {
    if (echart === undefined || echart === null) {
        return;
    }
    if (echart.addResizeEvent && echart.resizeFunc !== undefined) {
        window.removeEventListener('resize', echart.resizeFunc);
        echart.addResizeEvent = false;
        delete echart.resizeFunc;
    }
    if (echart.addLoadEvent && echart?.loadFunc !== undefined) {
        window.removeEventListener('load', echart.loadFunc);
        echart.addLoadEvent = false;
        delete echart.loadFunc;
    }
}

export const getDefaultChartOptions = (isDark: boolean): any => {
    return {
        tooltip: {
            backgroundColor: isDark ? '#2A2F37' : '#EBEFF6',
            textStyle: {
                color: isDark ? '#D2DCE9' : '#4E5865',
            },
            borderWidth: 0,
            padding: 16,
        },
        toolbox: {
            feature: {
                dataView: {
                    backgroundColor: isDark ? '#2A2F37' : '#EBEFF6',
                    textareaColor: isDark ? '#2A2F37' : '#EBEFF6',
                    textColor: isDark ? '#D2DCE9' : '#4E5865',
                    buttonColor: '#0077FF',
                },
            },
        },
    };
};
