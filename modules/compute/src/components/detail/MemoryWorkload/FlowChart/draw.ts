/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
*/
import * as d3 from 'd3';
import type { Igraph, Inode, InodePosition, IlinePosition, Ibox, IdrawGraph, IdrawNode, IdrawLine, IdrawRect, Iline, Irect, IrectPosition, Ixy, IdrawLabel } from './flowType';
import type { ImemoryData } from '../MemoryChart';
import type { Icondition } from '../Filter';

const cubeCore: Inode = {
    name: '',
    left: 0,
    container: [
        {
            x: 100,
            y: 0,
            width: 610,
            height: 310,
        },
    ],
    rect: [
        {
            name: 'L1',
            x: 140,
            y: 80,
            width: 80,
            height: 180,
            label: 'L1',
        },
        {
            left: 100,
            top: 15,
            width: 100,
            height: 50,
            label: 'L0A',
        },
        {
            top: 100,
            left: -100,
            width: 100,
            height: 50,
            label: 'L0B',
        },
        {
            top: -70,
            left: 100,
            width: 150,
            height: 90,
            label: 'Cube',
        },
        {
            top: -100,
            left: -125,
            width: 100,
            height: 50,
            label: 'L0C',
        },
    ],
    line: [
        {
            x: 0,
            y: 155,
            orient: 'right',
            length: 139,
            label: '0',
        },
        {
            x: 140,
            y: 180,
            length: 139,
            orient: 'left',
            label: '0',
            labelPosition: 'bottom',
        },
        {
            x: 220,
            y: 220,
            length: 99,
            orient: 'right',
            label: '0',
        },
        {
            x: 600,
            y: 125,
            length: 49,
            orient: 'top',
            label: '0',
            labelPosition: 'right',
        },
        {
            points: '0,280 365,280 365,250',
            label: '0',
            labelXy: { x: 320, y: 295 },
        },
        {
            points: '0,60 250,60 250,150 220,150 250,150 250,60 365,60 365,91',
            label: '0',
            labelXy: { x: 320, y: 50 },
        },
        {
            points: '420,115  450,115 450,170 450,220 420,220 450,220 450,170  520,170',
            label: '0',
            labelXy: { x: 475, y: 160 },
        },
        {
            points: '600,25 600,10 0,10',
            label: '0',
            labelXy: { x: 60, y: 25 },
        },
    ],
};
const vectorCore: Inode = {
    name: '',
    left: 0,
    top: 0,
    container: [
        {
            left: 100,
            top: 0,
            width: 610,
            height: 150,
        },
    ],
    rect: [
        {
            x: 140,
            y: 20,
            width: 150,
            height: 110,
            label: 'UB',
        },
        {
            top: 0,
            left: 120,
            width: 200,
            height: 110,
            label: 'Vec',
        },
    ],
    line: [
        {
            x: 0,
            top: '46%-10',
            length: 139,
            orient: 'right',
            label: '0',
        },
        {
            x: 140,
            top: '46%+10',
            length: 139,
            orient: 'left',
            label: '0',
            labelPosition: 'bottom',
        },
        {
            x: 290,
            top: '46%-10',
            length: 119,
            orient: 'right',
            label: '0',
        },
        {
            x: 410,
            top: '46%+10',
            length: 119,
            orient: 'left',
            label: '0',
            labelPosition: 'bottom',
        },
    ],
};
const cube: Igraph = [
    {
        name: 'hbm',
        x: 1,
        y: 1,
        rect: [
            {
                name: 'HBM',
                width: 80,
                height: 400,
                label: 'HBM',
            },
        ],
    },
    {
        name: 'l2catch',
        left: 0,
        rect: [
            {
                name: 'L2Catch',
                left: 80,
                width: 60,
                height: 400,
                label: 'L2 Catch',
            },
        ],
        line: [
            {
                x: 80,
                y: 85,
                orient: 'left',
                length: 79,
                label: 0,
            },
            {
                x: 0,
                y: 110,
                orient: 'right',
                length: 79,
                label: 0,
                labelPosition: 'bottom',
            },
        ],
    },
    cubeCore,
];
const vector: Igraph = [
    {
        name: 'hbm',
        x: 1,
        y: 1,
        rect: [
            {
                name: 'HBM',
                width: 80,
                height: 200,
                label: 'HBM',
            },
        ],
    },
    {
        name: 'l2catch',
        left: 0,
        rect: [
            {
                name: 'L2Catch',
                top: 0,
                left: 80,
                width: 60,
                height: 200,
                label: 'L2 Catch',
            },
        ],
        line: [
            {
                x: 80,
                y: 85,
                orient: 'left',
                length: 0,
                label: 0,
            },
            {
                x: 0,
                y: 110,
                orient: 'right',
                length: 79,
                label: 0,
                labelPosition: 'bottom',
            },
        ],
    },
    vectorCore,
];
const mix: Igraph = [
    {
        name: 'hbm',
        x: 1,
        y: 1,
        rect: [
            {
                name: 'HBM',
                width: 80,
                height: 650,
                label: 'HBM',
            },
        ],
    },
    {
        name: 'l2catch',
        left: 0,
        rect: [
            {
                name: 'L2Catch',
                left: 120,
                width: 60,
                height: 650,
                labels: [
                    { value: 'L2 Catch' },
                    { value: '' },
                    { value: 'Hit Ratio' },
                    { id: 'hitRatio', value: '0%' },

                ],
            },
        ],
        line: [
            {
                x: 120,
                top: '46%-10',
                orient: 'left',
                length: 119,
                label: '0',
                id: '1',
            },
            {
                x: 0,
                top: '46%+10',
                orient: 'right',
                length: 119,
                label: '0',
                labelPosition: 'bottom',
            },
        ],
    },
    cubeCore,
    { ...vectorCore, x: 260, y: 320 },
    { ...vectorCore, x: 260, y: 500 },
];
const flow: Record<string, Igraph> = { mix, cube, vector };

const defaultBox = {
    x1: 0,
    x2: 0,
    y1: 0,
    y2: 0,
    width: 0,
    height: 0,
};
function getDrawConfig(originData: Igraph): IdrawGraph {
    let lastNodeBox = JSON.parse(JSON.stringify(defaultBox));
    const data = originData.map((originNode, index) => {
        const node = { ...originNode, ...transformNode(originNode, lastNodeBox) };

        // container
        let lastContainerBox = JSON.parse(JSON.stringify(defaultBox));
        const container = (originNode.container ?? []).map(originContainer => {
            const rectPosition = transformRect(originContainer, lastContainerBox);
            lastContainerBox = {
                x1: rectPosition.x,
                y1: rectPosition.y,
                x2: rectPosition.x + rectPosition.width,
                y2: rectPosition.y + rectPosition.height,
                width: rectPosition.width,
                height: rectPosition.height,
            };
            const labels = transRectLabels(originContainer, rectPosition);
            return { ...originContainer, ...rectPosition, labels };
        });
        const allContainerBox = getAllRectBox(container);

        // rect
        let lastRectBox = defaultBox;
        const rect = (originNode.rect ?? []).map(originRect => {
            const rectPosition = transformRect(originRect, lastRectBox);
            lastRectBox = {
                x1: rectPosition.x,
                y1: rectPosition.y,
                x2: rectPosition.x + rectPosition.width,
                y2: rectPosition.y + rectPosition.height,
                width: rectPosition.width,
                height: rectPosition.height,
            };

            const labelXy = transRectLabel(originRect, rectPosition);
            const labels = transRectLabels(originRect, rectPosition);
            return { ...originRect, ...rectPosition, labelXy, labels };
        });
        const allRectBox = getAllRectBox(rect);
        // line
        const line = (originNode.line ?? []).map(originLine => {
            const { points } = transformLine(originLine, allRectBox);
            const labelXy = transLineLabel(originLine, points);
            return { ...originLine, points, labelXy };
        });
        const allLineBox = getAllLineBox(line);

        lastNodeBox = getNodeBox(node, [allContainerBox, allRectBox, allLineBox]);
        return { ...originNode, ...node, container, rect, line };
    });
    return data;
}
const transformNode = (origin: InodePosition, lastNodeBox: Ibox): IdrawNode => {
    const { x: originX, y: originY, left, top } = origin;
    let x;
    let y;
    if (originX !== undefined) {
        x = originX;
    } else if (left !== undefined) {
        x = lastNodeBox.x2 + transLeftTop(left, lastNodeBox.width);
    } else {
        x = 0;
    }
    if (originY !== undefined) {
        y = originY;
    } else if (top !== undefined) {
        y = lastNodeBox.y1 + transLeftTop(top, lastNodeBox.height);
    } else {
        y = 0;
    }
    return { x, y };
};
const transLeftTop = (lt?: number | string, length = 0): number => {
    if (typeof lt === 'number') {
        return lt;
    } else if (typeof lt === 'string') {
        return getPercentLength(lt, length);
    } else {
        return 0;
    }
};
const getPercentLength = (percent: string, length = 0): number => {
    const regPercent = /^-?(?<percent>-?0|100|(?<num>[1-9]?[0-9]))%$/; // '100%'
    const regMixPercent = /^(?<prcent>-?(?<percentNum>0|100|(?<num>[1-9]?[0-9]))%)[-+][0-9]{1,5}$/; // '100%-10'
    if (regPercent.test(percent)) {
        return Math.floor(length * Number(percent.split('%')[0]) / 100);
    } else if (regMixPercent.test(percent)) {
        const list = percent.split('%');
        return Math.floor(length * Number(list[0]) / 100) + Number(list[1]);
    } else {
        return 0;
    }
};
const transformRect = (origin: IrectPosition, lastRectBox: Ibox): IdrawRect => {
    const { x: originX, y: originY, left, top, width = 0, height = 0 } = origin;
    let x;
    let y;
    if (originX !== undefined) {
        x = originX;
    } else if (left !== undefined) {
        x = lastRectBox.x2 + transLeftTop(left, lastRectBox.width);
    } else {
        x = 0;
    }

    if (originY !== undefined) {
        y = originY;
    } else if (top !== undefined) {
        y = lastRectBox.y1 + transLeftTop(top, lastRectBox.height);
    } else {
        y = 0;
    }
    return { x, y, width, height };
};
const getAllRectBox = (allRect: IdrawRect[]): Ibox => {
    const box = {
        x1: Number.MAX_VALUE,
        y1: Number.MAX_VALUE,
        x2: 0,
        y2: 0,
    };
    allRect.forEach(item => {
        if (item.x < box.x1) {
            box.x1 = item.x;
        }
        if (item.y < box.y1) {
            box.y1 = item.y;
        }
        if (item.x + item.width > box.x2) {
            box.x2 = item.x + item.width;
        }
        if (item.y + item.height > box.y2) {
            box.y2 = item.y + item.height;
        }
    });
    return { ...box, width: box.x2 - box.x1, height: box.y2 - box.y1 };
};
const transRectLabel = (origin: Irect, rectPosition: IdrawRect): Ixy => {
    if (origin.labelXy !== undefined) {
        return origin.labelXy;
    }
    const { x, y, width, height } = rectPosition;
    return { x: x + (width / 2), y: y + (height * 0.46) };
};

const transRectLabels = (origin: Irect, rectPosition: IdrawRect): undefined | IdrawLabel[] => {
    const { labels } = origin;
    if (labels === undefined) {
        return labels;
    }
    const { x, y, width, height } = rectPosition;
    const centerX = x + (width * 0.5);
    const centerY = y + (height * 0.46);
    const length = labels.length;
    const baseY = centerY - Math.floor((length - 1) / 2 * 20);
    return labels.map((item, index) => ({
        ...item,
        x: centerX,
        y: baseY + (index * 20),
    }));
};
const transformLine = (origin: IlinePosition, box: Ibox): IdrawLine => {
    if (origin.points !== undefined) {
        return { points: origin.points };
    }
    const isFullXy = origin.x1 !== undefined && origin.x2 !== undefined && origin.x2 !== undefined && origin.y2 !== undefined;
    if (isFullXy) {
        return { points: `${origin.x1},${origin.y1} ${origin.x2}${origin.y2}` };
    }
    return transformLinePosition(origin, box);
};
const transformLinePosition = (origin: IlinePosition, box: Ibox): IdrawLine => {
    let x1;
    let y1;
    let x2;
    let y2;
    const { x, y, left, top, length = 0 } = origin;
    if (x !== undefined) {
        x1 = x;
    } else if (left !== undefined) {
        x1 = box.x2 + transLeftTop(left, box.width);
    } else {
        x1 = 0;
    }
    if (y !== undefined) {
        y1 = y;
    } else if (top !== undefined) {
        y1 = box.y1 + transLeftTop(top, box.height);
    } else {
        y1 = 0;
    }
    switch (origin.orient) {
        case 'left':
            x2 = x1 - length;
            y2 = y1;
            break;
        case 'right':
            x2 = x1 + length;
            y2 = y1;
            break;
        case 'top':
            x2 = x1;
            y2 = y1 - length;
            break;
        case 'bottom':
            x2 = x1;
            y2 = y1 + length;
            break;
        default:
            x2 = x1;
            y2 = y1;
            break;
    }
    return { points: `${x1},${y1} ${x2},${y2}` };
};
const transLineLabel = (origin: Iline, points: string): {x: number;y: number} => {
    if (origin.labelXy !== undefined) {
        return origin.labelXy;
    }

    const pointArr = points.split(' ').map(pItem => {
        const ord = pItem.split(',');
        return [Number(ord[0]), Number(ord[1])];
    });
    const lineOuter = { x1: Number.MAX_VALUE, y1: Number.MAX_VALUE, x2: 0, y2: 0 };
    pointArr.forEach(pItem => {
        const [x, y] = pItem;
        if (!isNaN(x) && x > lineOuter.x2) {
            lineOuter.x2 = x;
        }
        if (!isNaN(x) && x < lineOuter.x1) {
            lineOuter.x1 = x;
        }
        if (!isNaN(y) && y > lineOuter.y2) {
            lineOuter.y2 = y;
        }
        if (!isNaN(y) && y < lineOuter.y1) {
            lineOuter.y1 = y;
        }
    });
    const lablePosition = origin.labelPosition ?? 'top';
    const centerX = Math.floor((lineOuter.x1 + lineOuter.x2) / 2);
    const centerY = Math.floor((lineOuter.y1 + lineOuter.y2) / 2);
    let dx = 0;
    let dy = 0;
    switch (lablePosition) {
        case 'top':
            dy = -15;
            break;
        case 'bottom':
            dy = 15;
            break;
        case 'left':
            dx = -String(origin.label).length * 7;
            break;
        case 'right':
            dx = String(origin.label).length * 7;
            break;
        default:
            break;
    }
    return { x: centerX + dx, y: centerY + dy };
};
const getAllLineBox = (line: IdrawLine[]): Ibox => {
    const lineOuter = { x1: Number.MAX_VALUE, y1: Number.MAX_VALUE, x2: 0, y2: 0 };
    line.forEach(item => {
        const { points } = item;
        const pointArr = points.split(' ').map(pItem => {
            const ord = pItem.split(',');
            return [Number(ord[0]), Number(ord[1])];
        });
        pointArr.forEach(pItem => {
            const [x, y] = pItem;
            if (!isNaN(x) && x > lineOuter.x2) {
                lineOuter.x2 = x;
            }
            if (!isNaN(x) && x < lineOuter.x1) {
                lineOuter.x1 = x;
            }
            if (!isNaN(y) && y > lineOuter.y2) {
                lineOuter.y2 = y;
            }
            if (!isNaN(y) && y < lineOuter.y1) {
                lineOuter.y1 = y;
            }
        });
    });
    const { x1, x2, y1, y2 } = lineOuter;
    return { ...lineOuter, width: x2 - x1, height: y2 - y1 };
};
const getNodeBox = (node: Ixy, boxlist: Ibox[]): Ibox => {
    const nodeBox = { x1: Number.MAX_VALUE, y1: Number.MAX_VALUE, x2: 0, y2: 0 };
    boxlist.forEach(box => {
        if (nodeBox.x1 > box.x1) {
            nodeBox.x1 = box.x1;
        }
        if (nodeBox.x2 < box.x2) {
            nodeBox.x2 = box.x2;
        }
        if (nodeBox.y1 > box.y1) {
            nodeBox.y1 = box.y1;
        }
        if (nodeBox.y2 < box.y2) {
            nodeBox.y2 = box.y2;
        }
    });
    const { x, y } = node;
    const { x1, x2, y1, y2 } = nodeBox;
    return { x1: x + x1, x2: x + x2, y1: y + y1, y2: y + y2, width: x2 - x1, height: y2 - y1 };
};

const COLOR = {
    container: 'rgb(43,43,43)',
    rect: '#89c35e',
    rectStroke: '#696969',
    line: '#808080',
    label: '#fff',
    memory: '#40a9ff',
};

const drawNode = (svg: d3.Selection<d3.BaseType, unknown, HTMLElement, any>, data: IdrawGraph): d3.Selection<SVGGElement, IdrawNode, d3.BaseType, unknown> => {
    return svg.selectAll('g.node')
        .data(data)
        .enter()
        .append('g')
        .attr('class', 'node')
        .attr('transform', d => `translate(${d.x},${d.y})`);
};
const drawContainer = (nodes: d3.Selection<SVGGElement, IdrawNode, d3.BaseType, unknown>): void => {
    nodes.selectAll('rect.container')
        .data((d, i) => {
            return d.container ?? [];
        })
        .enter()
        .append('rect')
        .attr('class', 'container')
        .attr('x', d => d.x)
        .attr('y', d => d.y)
        .attr('width', d => d.width)
        .attr('height', d => d.height)
        .attr('rx', 1)
        .attr('ry', 1)
        .attr('fill', COLOR.container)
        .style('stroke', COLOR.rectStroke);
};

const drawRect = (nodes: d3.Selection<SVGGElement, IdrawNode, d3.BaseType, unknown>): void => {
    nodes.selectAll('rect.rect')
        .data((d, i) => {
            return d.rect ?? [];
        })
        .enter()
        .append('rect')
        .attr('class', 'rect')
        .attr('x', d => d.x)
        .attr('y', d => d.y)
        .attr('width', d => d.width)
        .attr('height', d => d.height)
        .attr('rx', 1)
        .attr('ry', 1)
        .attr('fill', d => ['HBM', 'L2Catch', 'L1'].includes(d.name ?? '') ? COLOR.memory : COLOR.rect)
        .style('stroke', COLOR.rectStroke);

    nodes.selectAll('text.rect-label')
        .data((d, i) => {
            return d.rect ?? [];
        })
        .enter()
        .append('text')
        .attr('class', 'rect-label')
        .attr('x', d => d.labelXy?.x ?? 0)
        .attr('y', d => d.labelXy?.y ?? 0)
        .attr('text-anchor', 'middle')
        .attr('dominant-baseline', 'middle')
        .text(d => d.label ?? '')
        .style('font-size', '14px')
        .style('fill', COLOR.label);
    nodes.selectAll('text.rect-labels')
        .data(d => {
            const allLabels = (d.rect ?? []).map(item => item.labels ?? []);
            return allLabels.flat();
        })
        .enter()
        .append('text')
        .attr('class', 'rect-labels')
        .attr('x', d => d.x ?? 0)
        .attr('y', d => d.y ?? 0)
        .attr('text-anchor', 'middle')
        .attr('dominant-baseline', 'middle')
        .text(d => d.value ?? '')
        .style('font-size', '14px')
        .style('fill', COLOR.label);
};

const drawLine = (nodes: d3.Selection<SVGGElement, IdrawNode, d3.BaseType, unknown>): void => {
    const glines = nodes.selectAll('g.line')
        .data((d) => {
            return d.line ?? [];
        })
        .enter()
        .append('g')
        .attr('class', 'line');
    glines.selectAll('polyline')
        .data((d, i) => {
            return [d];
        })
        .enter()
        .append('polyline')
        .attr('points', d => d.points)
        .attr('stroke', COLOR.line)
        .attr('stroke-width', '2px')
        .attr('fill', 'none')
        .attr('marker-end', 'url(#arrow)');
    glines.selectAll('text.line-label')
        .data((d, i) => {
            return [d];
        })
        .enter()
        .append('text')
        .attr('class', 'line-label')
        .attr('x', d => d.labelXy?.x ?? 0)
        .attr('y', d => d.labelXy?.y ?? 0)
        .attr('text-anchor', d => {
            switch (d.labelPosition) {
                case 'left':
                    return 'end';
                case 'right':
                    return 'start';
                default:
                    return 'middle';
            }
        })
        .attr('dominant-baseline', 'middle')
        .text(d => d.label ?? '')
        .style('font-size', '12px')
        .style('font-family', 'Micro Yahei')
        .style('fill', COLOR.label);
};

const addMarker = (svg: d3.Selection<d3.BaseType, unknown, HTMLElement, any>): void => {
    const defs = svg.append('defs');
    const arrowMarker = defs.append('marker')
        .attr('id', 'arrow')
        .attr('markerUnits', 'strokeWidth')
        .attr('markerWidth', '8')
        .attr('markerHeight', '8')
        .attr('viewBox', '0 0 12 12')
        .attr('refX', '8')
        .attr('refY', '5')
        .attr('orient', 'auto');
    const arrowPath = 'M 0 0 L 10 5 L 0 10 z';
    arrowMarker.append('path')
        .attr('d', arrowPath)
        .attr('fill', COLOR.line);
};
export const clear = (svg: d3.Selection<d3.BaseType, unknown, HTMLElement, any>): void => {
    svg.selectAll('*').remove();
};
export const drawGraph = (svg: d3.Selection<d3.BaseType, unknown, HTMLElement, any>, config: Igraph): void => {
    clear(svg);
    const graph = getDrawConfig(config);
    const nodes = drawNode(svg, graph);
    drawContainer(nodes);
    drawRect(nodes);
    drawLine(nodes);
    addMarker(svg);
};

export const drawFlowChart = (svg: d3.Selection<d3.BaseType, unknown, HTMLElement, any>, data: ImemoryData & Icondition): void => {
    const { blockType } = data;
    const graphConfig = flow[blockType];
    drawGraph(svg, graphConfig);
    updateData(svg, data);
};

export const updateData = (svg: d3.Selection<d3.BaseType, unknown, HTMLElement, any>, data: ImemoryData & Icondition): void => {
    const { showAs, memoryUnit } = data;
    const dic: Record<string, string> = {};
    memoryUnit.forEach(unit => {
        dic[unit.memoryPath] = unit[showAs];
    });
    svg.selectAll('text.line-label')
        .text(d => {
            const id = (d as IdrawLine).id ?? '';
            return dic[id] ?? '0';
        });
    svg.selectAll('text.rect-labels')
        .text((d: any) => {
            if (d.id === 'hitRatio') {
                return data.L2catch.hitRatio;
            }
            return d.value ?? '';
        });
};
