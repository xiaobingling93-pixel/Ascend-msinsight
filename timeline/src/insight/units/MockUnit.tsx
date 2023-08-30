import styled from '@emotion/styled';
import { runInAction } from 'mobx';
import React from 'react';
import { registerCrossUnitRenderer } from '../../components/charts/ChartInteractor';
import { drawRoundedRect } from '../../components/charts/common';
import { SimpleTabularDetail } from '../../components/details/SimpleDetail';
import { StackStatusData, StatusData } from '../../entity/chart';
import { chart, detail, DetailDescriptor, InsightUnit, on, unit, UnitHeight } from '../../entity/insight';
import { Session } from '../../entity/session';
import { toPercent } from '../../utils/humanReadable';
import { SliceRight } from './AscendUnit';
import { colorPalette } from './utils';

interface ThreadInfo {
    tid: number;
    name: string;
}

const Category = (function () {
    const categories: Record<0 | 1 | 2, { name: string; color: string; background: string }> = {
        0: { name: 'Native Lib', color: '#5090fc', background: '#1f2531' },
        1: { name: 'User Function', color: '#5c9d5d', background: '#212820' },
        2: { name: 'NAPI', color: '#d56c40', background: '#423733' },
    };
    const StyledLabel = styled.span<{ data: FunctionCall }>`
        color: ${p => categories[p.data.category].color};
        background-color: ${p => categories[p.data.category].background};
        border-radius: 4px;
        padding: 2px 6px;
    `;
    const Category = (props: { data: FunctionCall}): JSX.Element => <StyledLabel data={props.data}>
        { categories[props.data.category].name }
    </StyledLabel>;
    return Category;
})();

const mockRandomDetail = detail({
    columns: [
        [ 'Symbol Name', data => data.name, 'max-content' ],
        [ 'Weight', data => `${data.total}ms`, 100 ],
        [ '%', data => `${toPercent(data.total / 100, 1)}`, 100 ],
        [ 'Self', data => `${data.self}ms`, 100 ],
        [ '%', data => `${toPercent(data.self / 100, 1)}`, 100 ],
        [ 'Category', data => <Category data={data}/>, 200 ],
    ],
    fetchData: fetchDetail,
    rowKey: data => `${data.id} ${data.name} ${data.total}`,
    more: {
        field: 'posTicks',
        columns: [
            [ 'Line', data => `${data.line}`, 200, 'left' ],
            [ 'Ticks', data => `${data.ticks}`, 100, 'right' ],
        ],
    },
});

type FunctionCall = {
    id: string;
    name: string;
    total: number;
    self: number;
    category: 0 | 1 | 2;
    totalPercent: number;
    selfPercent: number;
    posTicks: PosTick[];
    source?: string;
    children?: FunctionCall[];
};

type PosTick = {
    line: number;
    ticks: number;
};

const rand = (lo: number, hi: number): number => {
    return lo + Math.floor(Math.random() * (hi - lo));
};

async function fetchDetail(session: Session, threadInfo: ThreadInfo | undefined): Promise<FunctionCall[]> {
    const length = 1000;
    let id = 0;
    const generateFunc = (): FunctionCall => {
        const func: FunctionCall = {
            id: `${id}`,
            name: `Function #${id} ${threadInfo ? '/' + threadInfo.name : ''}`,
            total: rand(50, 100),
            self: rand(0, 50),
            category: rand(0, 3) as 0 | 1 | 2,
            totalPercent: Math.random(),
            selfPercent: rand(0, 100),
            posTicks: Array(rand(0, 8)).fill(0).map((_, i) => ({ line: i, ticks: rand(1, 10) })),
        };
        id++;
        return func;
    };
    const funcs = Array(length).fill(null).map((_, i) => {
        const func = generateFunc();
        if (i % 3 === 1) {
            func.children = Array(i).fill(null).map(() => generateFunc());
        }
        return func;
    });
    return new Promise(resolve => {
        setTimeout(() => {
            resolve(funcs);
        }, 400);
    });
}

const MockThreadUnit = unit<ThreadInfo>({
    name: 'mock thread',
    tag: 'Insight',
    renderInfo: (session, threadInfo) => `Mock Thread #${threadInfo.tid}`,
    chart: [ chart({
        type: 'filledLine',
        height: UnitHeight.UPPER,
        mapFunc: async () => [
            [ 0, 100 ], [ 1000, 100 ], [ 2000, 100 ], [ 3000, 100 ], [ 4000, 100 ], [ 5000, 100 ], [ 6000, 100 ], [ 7000, 100 ], [ 8000, 100 ], [ 9000, 100 ], [ 10000, 100 ], [ 11000, 100 ], [ 12000, 100 ], [ 13000, 100 ], [ 14000, 100 ], [ 15000, 100 ], [ 16000, 100 ], [ 17000, 100 ], [ 18000, 100 ], [ 19000, 100 ], [ 20000, 100 ],
        ],
        config: {
            palette: ['gold'],
        },
    }), chart({
        type: 'stackedBar',
        height: UnitHeight.UPPER,
        mapFunc: async (session: Session) => {
            const ret = Array(19).fill(1000).map((data, index) => {
                return {
                    timestamp: data * (index + 1),
                    values: [data * 100],
                };
            });
            return ret;
        },
        config: {
            barWidth: 24,
            radius: 4,
            yScaleType: 'SymLog',
            palette: ['#5AADA0'],
        },
    }) ],
    detail: mockRandomDetail,
});

export const MockUnit = unit({
    name: 'mock process',
    tag: 'Insight',
    chart: chart({
        type: 'filledLine',
        height: UnitHeight.UPPER,
        mapFunc: async () => [],
        config: {
            palette: [],
        },
    }),
    // detail: mockRandomDetail,
    bottomPanelRender: () => ({
        Detail: ({ session, height }) => <SimpleTabularDetail detail={mockRandomDetail as DetailDescriptor<unknown>} session={session} height={height}/>,
    }),
    spreadUnits: on(
        'create',
        async (self: InsightUnit): Promise<void> => {
            const threads = await new Promise<ThreadInfo[]>((resolve) => {
                setTimeout(() => {
                    const threads = Array(5).fill(0).map((_, i) => ({ tid: i, name: `Thread #${i}` }));
                    resolve(threads);
                }, 200);
            });
            self.children = threads.map(it => new MockThreadUnit(it));
        }),
});

const renderRadiusBorder = (topLeft: number, topRight: number, bottomRight: number, bottomLeft: number, ctx: CanvasRenderingContext2D): void => {
    const halfLine = 1;
    ctx.lineWidth = halfLine * 2;
    let width = bottomRight;
    width = Math.max(1, Math.floor(width));
    const radius = width >= 8 ? 4 : width / 2;
    if (radius < 1) {
        ctx.strokeRect(topLeft, topRight + halfLine, bottomRight, bottomLeft - halfLine - 1);
    } else {
        drawRoundedRect([ topLeft, topRight + halfLine, bottomRight, bottomLeft - halfLine - 1 ], ctx, radius);
        ctx.stroke();
    }
};

export const MockStackStatus = unit<number>({
    name: 'MockStackStatus',
    tag: 'MockStackStatus',
    pinType: 'move',
    chart: chart({
        config: { rowHeight: 20 },
        height: 60,
        mapFunc: async (session, metadata: unknown) => {
            const mockData = Array(3).fill(0).map((it, index) => Array(8).fill(0).map((_, idx) => ({
                startTime: 1e9 * idx * 2,
                duration: 1e9,
                type: `${index}-idx`,
                name: `${index}-idx`,
                color: colorPalette[index],
                depth: index,
                metadata: metadata,
            })));
            return mockData;
        },
        renderTooltip: undefined,
        type: 'stackStatus',
        onClick: (data, session, metadata) => {
            session.locateUnit = {
                target: (unit) => unit.metadata === 20,
                onSuccess: () => {
                    if (data === undefined) { return; }
                    session.domainRange = { domainStart: data.startTime - 1e9, domainEnd: data.startTime + data.duration + 1e9 };
                },
            };
        },
        decorator: (session, metadata) => {
            const selectedData = session.selectedData as { startTime: number; duration: number; type: string; name: string; color: string; depth: number; metadata: number } | undefined;
            return {
                action: (handle, xScale, yScale, theme) => {
                    const ctx = handle.context;
                    if (ctx === null || selectedData === undefined || metadata !== selectedData?.metadata) {
                        return;
                    }
                    ctx.strokeStyle = 'red';
                    renderRadiusBorder(xScale(selectedData.startTime), yScale(selectedData.depth),
                        xScale(selectedData.duration + selectedData.startTime) - xScale(selectedData.startTime), yScale(1), ctx);
                },
                triggers: [session.selectedData],
            };
        },
    }),
});

export const ForegroundUnit = unit({
    name: 'Foreground',
    tag: 'Insight',
    chart: chart({
        type: 'status',
        height: UnitHeight.STANDARD,
        mapFunc: async (session: Session) => {
            return [
                {
                    startTime: 0,
                    duration: 1000,
                    name: 'app1',
                    type: 'type1',
                    color: '#5AADA07F',
                },
                {
                    startTime: 2000,
                    duration: 300,
                    name: 'app2',
                    type: 'type2',
                    color: '#D1A8387F',
                },
                {
                    startTime: 3000,
                    duration: 643,
                    name: 'app3',
                    type: 'type3',
                    color: '#FFFFFF4C',
                },
                {
                    startTime: 4423,
                    duration: 1000,
                    name: 'app4',
                    type: 'type4',
                    color: '#D1A838CC',
                },
            ];
        },
        config: { },
    }),
});

export const StackedBarUnit = unit({
    name: 'StackedBar',
    tag: 'StackedBar',
    chart: chart({
        type: 'stackedBar',
        height: UnitHeight.UPPER,
        mapFunc: async (session: Session) => {
            let increase = 60;
            let height = 0;
            const ret = Array(24).fill(1000).map((data, index) => {
                index === 12 && (increase *= -1);
                height += increase;
                return {
                    timestamp: data * (index + 1),
                    values: Array(6).fill(height / 6),
                };
            });
            return ret;
        },
        config: {
            barWidth: 24,
            radius: 4,
            yScaleType: 'Linear',
            palette: [
                '#4183a2',
                '#549251',
                '#b09239',
                '#bb5f43',
                '#af4c78',
                '#5953bd',
            ],
        },
    }),
});

type MockedStatusData = {
    startTime: number;
    duration: number;
    scheduleTime: number;
    scheduledBy: number;
    name: string;
    metadata: number;
    type: string;
    color: string;
};

const unitsCount = 8;

const mockedChartData = (session: Session, index: number): MockedStatusData[] => {
    const data = (function(totalUnits: number): MockedStatusData[][] {
        const { domainStart, domainEnd } = session.domainRange;
        const end = Math.min(session.endTimeAll ?? 0, domainEnd);
        return Array(totalUnits).fill(null).map((_, unitIndex) => {
            return Array(20).fill(null).map((_, i) => ({
                startTime: i * (end - domainStart) / 20,
                duration: (end - domainStart) / 20,
                scheduleTime: (i - 0.5) * (end - domainStart) / 20,
                scheduledBy: (unitIndex + 2) % unitsCount,
                metadata: unitIndex,
                name: `lane${unitIndex} ${i}`,
                type: `${i % 5}`,
                color: i % 2 === 0 ? 'blue' : 'green',
            }));
        });
    }(unitsCount));
    return data[index];
};

export const MockedStatusUnit = unit({
    name: 'mocked status unit',
    spreadUnits: on(
        'create',
        async (self, _) => {
            self.children = Array(unitsCount).fill(null).map((_, i) => new MockedStatusSubUnit(i));
        }),
});

const MockedStatusSubUnit = unit<number>({
    name: 'mocked status sub unit',
    chart: chart({
        type: 'status',
        height: UnitHeight.UPPER,
        mapFunc: async (session: Session, metadata: unknown): Promise<StatusData[]> => {
            return new Promise(resolve => { resolve(mockedChartData(session, metadata as number)); });
        },
        config: {},

        onHover: (data, session) => {
            // hover时将对应的数据点记录在共享状态中
            // 本泳道设置且观察（见下）sharedState中的mockStatusHovered字段，其他类的泳道可以用不同的key
            runInAction(() => {
                session.sharedState.mockStatusHovered = data;
            });
        },
        onClick: (data, session) => {
            runInAction(() => {
                session.selectedData = data;
            });
        },
        decorator: (session: Session, metadata: unknown) => ({
            // 自定义绘图，这些逻辑发生在chart的正常逻辑之后，是覆盖在原有图像内容之上的，所以叫decorator
            // 这部分绘制的内容取决于泳道自身的业务逻辑，不适合放在chart组件内，所以单拎出来
            // 如果要绘制的内容可能需要取决于其他泳道或全局状态，则需借助session进行状态共享
            action: (handle, xScale, yScale) => {
                // 利用handle中提供的接口找到泳道内所有匹配的数据点并渲染高亮色
                // session.sharedState是一个由mobx管理的observable对象，其内的字段可以由业务按需任意设置，唯一需要注意的是避免根别的unit撞key
                const hoveredData = session.sharedState.mockStatusHovered as MockedStatusData | undefined;
                if (hoveredData) {
                    const type = hoveredData.type;
                    const data = handle.findAll(it => it.type.localeCompare(type) !== 0).map(it => ({ ...it, color: 'rgba(255, 255, 255, 0.5)' }));
                    // status chart中的data内带有颜色信息，所以可以构造一组仅颜色不同的新data，覆盖画在原图上，实现高亮效果
                    handle.draw(data, xScale, yScale);
                }

                // 渲染click装饰，在渲染hover高亮之后，因为click装饰绘制的内容覆盖在hover高亮之上
                const ctx = handle.context;
                const selectedData = session.selectedData as MockedStatusData | undefined; // should check data type
                if (ctx !== null && selectedData !== undefined) {
                    if (selectedData.metadata === metadata) {
                        // 来自本泳道点击的数据，给数据描边+画线
                        const halfLine = 2;
                        ctx.lineWidth = halfLine * 2;
                        ctx.strokeRect(xScale(selectedData.startTime), yScale(0) + halfLine, xScale(selectedData.duration), yScale(1) - halfLine - 1);
                        ctx.fillStyle = 'black';
                        ctx.fillRect(xScale(selectedData.scheduleTime), yScale(0.65),
                            xScale(selectedData.startTime) - xScale(selectedData.scheduleTime), yScale(0.08));
                    } else if (selectedData.scheduledBy === metadata) {
                        // 来自其他泳道点击，但跟本泳道相关的数据，在数据之前一定位置画标记
                        const x = xScale(selectedData.scheduleTime);
                        const y = yScale(0.5);
                        const offset = yScale(0.25);
                        ctx.beginPath();
                        ctx.fillStyle = 'black';
                        ctx.moveTo(x, y - offset);
                        ctx.lineTo(x + offset, y);
                        ctx.lineTo(x, y + offset);
                        ctx.lineTo(x - offset, y);
                        ctx.closePath();
                        ctx.fill();
                    }
                }
            },
            // 这些对象变化，会触发这个泳道chart重绘，基本上应该列举action中访问过的session.sharedState内的所有字段
            triggers: [
                session.sharedState.mockStatusHovered,
                session.selectedData,
            ],
        }),
    }),
});

registerCrossUnitRenderer({
    action: (ctx, session, xScale) => {
        const selectedData = session.selectedData as MockedStatusData | undefined; // should filter on data type
        if (ctx !== null && selectedData !== undefined) {
            ctx.fillStyle = 'black';
            ctx.fillRect(xScale(selectedData.scheduleTime) - 1, 0, 2, 9999);
        }
    },
    triggers: session => [session.selectedData],
});
