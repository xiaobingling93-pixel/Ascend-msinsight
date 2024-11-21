/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */
import * as echarts from 'echarts';
import type { EChartsType } from 'echarts';
import type { Theme } from '@emotion/react';
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
    resizeFunc?: () => void;
    loadFunc?: () => void;
}
interface Listenter {
    id: string;
    type: string;
    value: () => void;
}
let listenerList: Listenter[] = [];
export function getAdaptiveEchart(chartDom: HTMLElement, theme?: string | object | null, opts?: any): EChartsType {
    let chart: EChartsType;
    if (echarts.getInstanceByDom(chartDom)) {
        chart = echarts.getInstanceByDom(chartDom) as EChartsType;
    } else {
        chart = echarts.init(chartDom, theme, opts);
        addResizeEvent(chart);
    }
    return chart;
}
export function addResizeEvent(echart?: EChartsTypeExtends): void {
    if (echart === undefined || echart === null) {
        return;
    }
    const domId = echart.getDom().id;
    if (!echart.resizeFunc) {
        const resizeFunc = (): void => {
            if (checkDomDisplay(echart.getDom())) {
                echart.resize();
            }
        };
        window.addEventListener('resize', resizeFunc);
        echart.resizeFunc = resizeFunc;
        if (domId !== '') {
            listenerList.push({
                id: domId,
                type:'resize',
                value: resizeFunc,
            });
        }
    }
    if (!echart.loadFunc) {
        const loadFunc = (): void => {
            if (checkDomDisplay(echart.getDom())) {
                echart.resize();
            }
        };
        window.addEventListener('load', loadFunc);
        echart.loadFunc = loadFunc;
        if (domId !== '') {
            listenerList.push({
                id: domId,
                type:'load',
                value: loadFunc,
            });
        }
    }
}
export function removeResizeEvent(echart?: EChartsTypeExtends): void {
    if (echart === undefined || echart === null) {
        return;
    }
    if (echart?.resizeFunc !== undefined) {
        window.removeEventListener('resize', echart.resizeFunc);
        delete echart.resizeFunc;
    }
    if (echart?.loadFunc !== undefined) {
        window.removeEventListener('load', echart.loadFunc);
        delete echart.loadFunc;
    }
}
export function disposeAdaptiveEchart(chartDom: HTMLElement): void {
    try {
        const listeners = listenerList.filter(item => item.id === chartDom.id);
        listeners.forEach(listener => {
            window.removeEventListener(listener.type, listener.value);
        });
        listenerList = listenerList.filter(item => item.id !== chartDom.id);
    } catch {
        // 不影响后面的，继续
    }
    const chart = echarts.getInstanceByDom(chartDom);
    removeResizeEvent(chart);
    chart?.dispose();
}

export const getDefaultChartOptions = (isDark?: boolean): any => {
    return {
        textStyle: {
            fontFamily: '\'Inter\', -apple-system, BlinkMacSystemFont, \'Segoe UI\', Roboto, Oxygen, Ubuntu, Cantarell, \'Fira Sans\', \'Droid Sans\', sans-serif',
        },
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
export const getLegendStyle = (theme: Theme): any => {
    return {
        textStyle: {
            color: theme.textColorTertiary,
        },
        pageTextStyle: {color: theme.textColorTertiary},
        pageIconColor: theme.primaryColorLight2,
        pageIconInactiveColor: theme.disableButtonBackgroundColor,
    };
};
