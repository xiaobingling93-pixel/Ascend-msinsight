/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
*/
import * as d3 from 'd3';
import type { TFunction } from 'i18next';
import type { Igraph, Inode, InodePosition, IlinePosition, Ibox, IdrawGraph, IdrawNode, IdrawLine, IdrawRect, Iline, Irect, IrectPosition, Ixy, IdrawLabel } from './flowType';
import type { ImemoryData, ImemoryUnit } from '../MemoryChart';
import type { Icondition } from '../Filter';
import { getFormatNum, getFormatNumReturnEmpty } from '../../../Common';
import { CompareData } from '../../../../utils/interface';

const bodyStyle = window.getComputedStyle(document.body);
const fontFamily = bodyStyle.fontFamily;

const cubeCore: Inode = {
    name: '',
    left: -15,
    top: 20,
    container: [
        {
            x: 125,
            y: -20,
            width: 610,
            height: 330,
        },
    ],
    rect: [
        {
            name: 'L1',
            x: 140,
            y: 105,
            width: 80,
            height: 170,
            label: 'L1',
        },
        {
            left: 100,
            top: 20,
            width: 100,
            height: 50,
            label: 'L0A',
        },
        {
            top: 90,
            left: -100,
            width: 100,
            height: 50,
            label: 'L0B',
        },
        {
            top: -80,
            left: 100,
            width: 150,
            height: 120,
            labels: [
                { value: 'Cube' },
                { value: 'Ratio:', x: 575, y: 215 },
                { id: 'cubeRatio', value: '0%', x: 605, y: 215, position: 'right' },
                { id: 'cubeRatioBaseline', value: '' },
            ],
        },
        {
            top: -110,
            left: -125,
            width: 100,
            height: 50,
            label: 'L0C',
        },
    ],
    line: [
        {
            id: 'L2_TO_L1',
            label: 'L2_TO_L1',
            x: 0,
            y: 155,
            orient: 'right',
            length: 138,
        },
        {
            id: 'L1_TO_L2',
            label: 'L1_TO_L2',
            x: 140,
            y: 180,
            length: 138,
            orient: 'left',
            labelPosition: 'bottom',
        },
        {
            id: 'GM_OR_L1_TO_L0A',
            label: 'GM_OR_L1_TO_L0A',
            points: '0,80 250,80 250,150 220,150 318,150',
            labelXy: { x: 265, y: 165 },
        },
        {
            id: 'GM_OR_L1_TO_L0B',
            label: 'GM_OR_L1_TO_L0B',
            points: '0,290 250,290 250,240 220,240 250,240 318,240',
            labelXy: { x: 265, y: 228 },
        },
        {
            id: 'L0A_TO_CUBE',
            label: 'L0A_TO_CUBE',
            x: 420,
            y: 150,
            orient: 'right',
            length: 97,
            labelPosition: 'bottom',
        },
        {
            id: 'L0B_TO_CUBE',
            label: 'L0B_TO_CUBE',
            x: 420,
            y: 240,
            orient: 'right',
            length: 97,
        },
        {
            id: 'CUBE_TO_L0C',
            label: 'CUBE_TO_L0C',
            x: 590,
            y: 135,
            length: 58,
            orient: 'top',
            labelPosition: 'left',
        },
        {
            id: 'L0C_TO_CUBE',
            label: 'L0C_TO_CUBE',
            x: 610,
            y: 75,
            length: 58,
            orient: 'bottom',
            labelPosition: 'right',
        },
        {
            id: 'L0C_TO_L1',
            label: 'L0C_TO_L1',
            points: '545,65 180,65 180,102',
            labelXy: { x: 210, y: 50 },
        },
        {
            id: 'L0C_TO_L2',
            label: 'L0C_TO_L2',
            points: '600,25 600,10 1,10',
            labelXy: { x: 60, y: 25 },
        },
    ],
};
const cubeCoreV2: Inode = {
    ...cubeCore,
    line: [...(cubeCore.line ?? []).filter(lineItem => !['GM_OR_L1_TO_L0A', 'GM_OR_L1_TO_L0B'].includes(lineItem.id as string)),
        {
            id: 'GM_TO_L0A_AIC',
            label: 'GM_TO_L0A_AIC',
            points: '0,80 370,80 370,123',
            labelXy: { x: 270, y: 95 },
        },
        {
            id: 'GM_TO_L0B_AIC',
            label: 'GM_TO_L0B_AIC',
            points: '0,290 370,290 370,267',
            labelXy: { x: 270, y: 280 },
        },
        {
            id: 'L1_TO_L0A_AIC',
            label: 'L1_TO_L0A_AIC',
            x: 220,
            y: 150,
            orient: 'right',
            length: 98,
            labelXy: { x: 265, y: 165 },
        },
        {
            id: 'L1_TO_L0B_AIC',
            label: 'L1_TO_L0B_AIC',
            x: 220,
            y: 240,
            orient: 'right',
            length: 98,
            labelXy: { x: 265, y: 228 },
        },
    ],
};
const vectorCore: Inode = {
    name: '',
    left: -15,
    top: 0,
    container: [
        {
            left: 125,
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
            name: 'UB',
        },
        {
            top: 0,
            left: 120,
            width: 200,
            height: 110,
            name: 'Vector',
            labels: [
                { value: 'Vector' },
                { value: 'Ratio:', x: 490, y: 95 },
                { id: 'vectorRatio', value: '0%', x: 520, y: 95, position: 'right' },
                { id: 'vectorRatioBaseline', value: '' },
            ],
        },
    ],
    line: [
        {
            id: 'L2_TO_UB',
            label: 'L2_TO_UB',
            x: 0,
            top: '46%-10',
            length: 138,
            orient: 'right',
        },
        {
            id: 'UB_TO_L2',
            label: 'UB_TO_L2',
            x: 140,
            top: '46%+10',
            length: 138,
            orient: 'left',
            labelPosition: 'bottom',
        },
        {
            id: 'UB_TO_VEC',
            label: 'UB_TO_VEC',
            x: 290,
            top: '46%-10',
            length: 118,
            orient: 'right',
        },
        {
            id: 'VEC_TO_UB',
            label: 'VEC_TO_UB',
            x: 410,
            top: '46%+10',
            length: 118,
            orient: 'left',
            labelPosition: 'bottom',
        },
    ],
};
const vectorCore2: Inode = {
    ...vectorCore,
    rect: (vectorCore.rect ?? []).map(item => {
        if (item.name === 'Vector') {
            return {
                ...item,
                labels: [
                    { value: 'Vector' },
                    { value: 'Ratio:', x: 490, y: 95 },
                    { id: 'vector1Ratio', value: '0%', x: 520, y: 95, position: 'right' },
                    { id: 'vector1RatioBaseline', value: '' },
                ],
            };
        }
        return item;
    }),
    line: (vectorCore.line ?? []).map(item => ({ ...item, id: `${item.id}_2`, label: `${item.label}_2` })),
};
const mixCore: Inode = {
    name: '',
    left: -15,
    top: -10,
    rect: [
        {
            name: 'L1',
            x: 100,
            y: 0,
            width: 80,
            height: 170,
            label: 'L1',
        },
        {
            left: 100,
            top: 20,
            width: 100,
            height: 50,
            label: 'L0A',
        },
        {
            top: 90,
            left: -100,
            width: 100,
            height: 50,
            label: 'L0B',
        },
        {
            top: -80,
            left: 100,
            width: 150,
            height: 120,
            labels: [
                { value: 'Cube' },
                { value: 'Ratio:', x: 535, y: 110 },
                { id: 'cubeRatio', value: '0%', x: 565, y: 110, position: 'right' },
                { id: 'cubeRatioBaseline', value: '' },
            ],
        },
        {
            top: 15,
            left: 100,
            width: 100,
            height: 50,
            label: 'L0C',
        },
        {
            top: 120,
            left: -350,
            width: 350,
            height: 50,
            labels: [
                { value: 'Vector', x: 655, y: 180 },
                { value: 'Ratio:', x: 635, y: 200 },
                { id: 'vectorRatio', x: 665, y: 200, value: '0%', position: 'right' },
                { id: 'vectorRatioBaseline', value: '' },
            ],
            name: 'Vector',
        },
        {
            top: 121,
            left: -649,
            width: 650,
            height: 100,
            label: 'UB',
            name: 'UB',
        },
    ],
    line: [
        {
            id: 'L2_OR_UB_TO_L1',
            label: 'L2_OR_UB_TO_L1',
            x: 0,
            y: 85,
            orient: 'right',
            length: 99,
        },
        {
            id: 'L2_OR_UB_TO_L1_append',
            label: '',
            points: '200,285 200,230 50,230 50,85 99,85',
        },
        {
            id: 'L2_OR_L1_TO_UB',
            x: 0,
            y: 310,
            orient: 'right',
            length: 179,
            label: 'L2_OR_L1_TO_UB',
            labelPosition: 'bottom',
        },
        {
            id: 'L2_OR_L1_TO_UB_append',
            label: '',
            points: '140,170 140,310 180,310',
        },
        {
            id: 'UB_TO_L2',
            label: 'UB_TO_L2',
            x: 179,
            y: 360,
            orient: 'left',
            length: 179,
            labelPosition: 'bottom',
        },
        {
            id: 'L1_TO_L0A',
            label: 'L1_TO_L0A',
            x: 180,
            y: 45,
            orient: 'right',
            length: 99,
        },
        {
            label: 'L2_OR_L1_TO_L0B',
            id: 'L2_OR_L1_TO_L0B',
            x: 180,
            y: 135,
            orient: 'right',
            length: 99,
        },
        {
            id: 'L2_OR_L1_TO_L0B_append',
            points: '0,210 200,210 200,135 279,135',
            label: '',
        },
        {
            id: 'L0A_TO_CUBE',
            label: 'L0A_TO_CUBE',
            x: 380,
            y: 45,
            orient: 'right',
            length: 99,
        },
        {
            id: 'L0B_TO_CUBE',
            label: 'L0B_TO_CUBE',
            x: 380,
            y: 135,
            orient: 'right',
            length: 99,
        },
        {
            id: 'CUBE_TO_L0C',
            label: 'CUBE_TO_L0C',
            x: 630,
            y: 70,
            orient: 'right',
            length: 99,
        },
        {
            id: 'L0C_TO_VEC',
            label: 'L0C_TO_VEC',
            x: 790,
            y: 95,
            length: 69,
            orient: 'bottom',
            labelPosition: 'right',
        },
        {
            id: 'VEC_TO_L0C',
            label: 'VEC_TO_L0C',
            x: 770,
            y: 165,
            length: 69,
            orient: 'top',
            labelPosition: 'left',
        },
        {
            id: 'VEC_TO_UB',
            label: 'VEC_TO_UB',
            x: 590,
            y: 215,
            length: 69,
            orient: 'bottom',
            labelPosition: 'right',
        },
        {
            id: 'UB_TO_VEC',
            label: 'UB_TO_VEC',
            x: 570,
            y: 285,
            length: 69,
            orient: 'top',
            labelPosition: 'left',
        },
    ],
};
const common: Inode[] = [
    {
        name: 'GM',
        x: 1,
        y: 1,
        container: [
            {
                width: 340,
                height: 660,
            },
        ],
        rect: [
            {
                left: 15,
                top: 15,
                name: 'HBM',
                width: 120,
                height: 630,
                label: 'Memory On Chip',
            },
            {
                name: 'L2Catch',
                left: 120,
                top: 0,
                width: 70,
                height: 630,
                labels: [
                    { value: 'L2 Cache' },
                    { value: '' },
                    { value: 'Hit Rate:' },
                    { id: 'hitRatio', value: '0%' },
                    { id: 'hitRatioBaseline', value: '' },
                ],
            },
        ],
        line: [
            {
                id: 'HBM_TO_L2',
                label: 'HBM_TO_L2',
                x: 135,
                top: '46%-10',
                orient: 'right',
                length: 117,
            },
            {
                id: 'L2_TO_HBM',
                label: 'L2_TO_HBM',
                x: 255,
                top: '46%+10',
                orient: 'left',
                length: 117,
                labelPosition: 'bottom',
            },
        ],
    },
];
const cube: Igraph = [
    ...common.map(item => ({
        ...item,
        rect: (item.rect ?? []).map(rectItem => ({ ...rectItem, height: 300 })),
        container: (item.container ?? []).map(containerItem => ({ ...containerItem, height: 330 })),
    })),
    cubeCore,
];
const cubeV2: Igraph = [
    ...common.map(item => ({
        ...item,
        rect: (item.rect ?? []).map(rectItem => ({ ...rectItem, height: 300 })),
        container: (item.container ?? []).map(containerItem => ({ ...containerItem, height: 330 })),
    })),
    cubeCoreV2,
];
const vector: Igraph = [
    ...common.map(item => ({
        ...item,
        rect: (item.rect ?? []).map(rectItem => ({ ...rectItem, height: 120 })),
        container: (item.container ?? []).map(containerItem => ({ ...containerItem, height: 150 })),
    })),
    vectorCore,
];
const mix: Igraph = [
    ...common,
    cubeCore,
    { ...vectorCore, x: 326, y: 345 },
    { ...vectorCore2, x: 326, y: 510 },
];
const mixV2: Igraph = [
    ...common,
    cubeCoreV2,
    { ...vectorCore, x: 291, y: 345 },
    { ...vectorCore2, x: 291, y: 510 },
];
const mix310: Igraph = [
    ...common.map(item => ({
        ...item,
        rect: (item.rect ?? []).map(rectItem => ({ ...rectItem, height: 350 })),
        container: (item.container ?? []).map(containerItem => ({ ...containerItem, height: 380 })),
    })),
    mixCore,
];

const flow: Record<string, Igraph> = {
    cube910: cube,
    cube910V2: cubeV2,
    vector910: vector,
    mix910: mix,
    mix910V2: mixV2,
    mix310,
};

enum Path {
    // 公共pipe
    HBM_TO_L2 = 0,
    L2_TO_HBM = 1,
    // cube Pipe
    L2_TO_L1 = 2,
    L1_TO_L2 = 3,
    GM_OR_L1_TO_L0A = 4,
    GM_OR_L1_TO_L0B = 5,
    L0A_TO_CUBE = 6,
    L0B_TO_CUBE = 7,
    CUBE_TO_L0C = 8,
    L0C_TO_L1 = 9,
    L0C_TO_L2 = 10,
    L0C_TO_CUBE = 11,
    // Vec Pipe
    L2_TO_UB = 12,
    UB_TO_L2 = 13,
    UB_TO_VEC = 14,
    VEC_TO_UB = 15,
    L2_TO_UB_2 = 16,
    UB_TO_L2_2 = 17,
    UB_TO_VEC_2 = 18,
    VEC_TO_UB_2 = 19,
    // 310P
    L2_OR_UB_TO_L1 = 20,
    L1_TO_L0A = 21,
    L2_OR_L1_TO_L0B = 22,
    VEC_TO_L0C = 23,
    L0C_TO_VEC = 24,
    L2_OR_L1_TO_UB = 25,
    GM_TO_L0A_AIC = 35,
    GM_TO_L0B_AIC = 36,
    L1_TO_L0A_AIC = 37,
    L1_TO_L0B_AIC = 38,
}

const defaultBox = {
    x1: 0,
    x2: 0,
    y1: 0,
    y2: 0,
    width: 0,
    height: 0,
};
function getDrawConfig(originData: Igraph, tDetails: TFunction): IdrawGraph {
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
            labels?.forEach(item => {
                item.value = item.value?.split(':').map(str => tDetails(str)).join(':');
            });
            const label = originRect.label !== undefined ? tDetails(originRect.label) : undefined;
            return { ...originRect, ...rectPosition, labelXy, labels, label };
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
    let baseY = centerY - Math.floor((length - 1) / 2 * 20);
    return labels.reduce<IdrawLabel[]>((pre, cur, index) => {
        let curY;
        if (cur.top !== undefined) {
            curY = y + transLeftTop(cur.top, height);
        } else {
            curY = baseY + 20;
        }
        const label = {
            x: centerX,
            y: curY,
            ...cur,
        };
        baseY = curY;
        pre.push(label);
        return pre;
    }, []);
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
            dx = -10;
            break;
        case 'right':
            dx = 10;
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

const colorConfig: Record<string, Record<string, any>> = {
    light: {
        container: '#f7fbff',
        rectStroke: '#d9d9d9',
        line: '#000000',
        rectLabel: '#fff',
        label: '#7b7a7a',
        memory: '#0062DC',
        rect: '#279C6E',
        aiv: '#75A105',
        range: ['#4c008a', '#c42627', '#ff5f03', '#ffc55b', '#fcc95e', '#e9e9e9'],
    },
    dark: {
        container: '#2b2b2b',
        rectStroke: '#696969',
        line: '#808080',
        rectLabel: '#fff',
        label: '#fff',
        memory: '#0062DC',
        rect: '#279C6E',
        aiv: '#75A105',
        range: ['#4c008a', '#c42627', '#ff5f03', '#ffc55b', '#fcc95e', '#ffffff'],
    },
};
let COLOR: any = colorConfig.light;

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
        .attr('fill', d => {
            const name = d.name ?? '';
            if (['HBM', 'L2Catch'].includes(name)) {
                return COLOR.memory;
            } else if (['UB', 'Vector'].includes(name)) {
                return COLOR.aiv;
            } else {
                return COLOR.rect;
            }
        })
        .style('stroke', COLOR.rectStroke);

    drawRectLabel(nodes);
};
const drawRectLabel = (nodes: d3.Selection<SVGGElement, IdrawNode, d3.BaseType, unknown>): void => {
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
        .style('fill', COLOR.rectLabel);
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
        .attr('text-anchor', d => {
            switch (d.position) {
                case 'left':
                    return 'end';
                case 'right':
                    return 'start';
                default:
                    return 'middle';
            }
        })
        .attr('dominant-baseline', 'middle')
        .text(d => d.value ?? '')
        .style('font-size', '14px')
        .style('fill', COLOR.rectLabel);
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
        .style('font-family', fontFamily)
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

const valueRange = [0, 20, 40, 60, 80, 100];
const gradientColors: Array<(val: number) => string> = [];
for (let i = 0; i < valueRange.length - 1; i++) {
    const lineScale = d3.scaleLinear()
        .domain([valueRange[i], valueRange[i + 1]])
        .range([0, 1]);
    const colorScale = d3.interpolate(COLOR.range[i], COLOR.range[i + 1]);
    gradientColors.push((d: number) => colorScale(lineScale(d)));
}
const computeColor = (d: number): string => {
    for (let i = 0; i < valueRange.length - 1; i++) {
        if (d >= valueRange[i] && d <= valueRange[i + 1]) {
            return gradientColors[i](d);
        }
    }
    return COLOR.line;
};

const addLegend = (svg: d3.Selection<d3.BaseType, unknown, HTMLElement, any>, title: string): void => {
    // 定义渐变色的比例尺
    const colorScale = d3.scaleLinear()
        .domain([0, COLOR.range.length - 1])
        .range([0, 1]);

    // 创建渐变色
    const defs = svg.append('defs');
    const gradient = defs.append('linearGradient')
        .attr('id', 'gradient')
        .attr('x1', '0%')
        .attr('y1', '100%')
        .attr('x2', '0%')
        .attr('y2', '0%');

    // 添加渐变色的颜色节点
    gradient.selectAll('stop')
        .data((COLOR.range as string[]) ?? [])
        .enter().append('stop')
        .attr('offset', (d, i) => colorScale(i))
        .attr('stop-color', d => d);

    const g = svg.append('g')
        .attr('class', 'legend')
        .attr('transform', 'translate(1215,10)');
    // 创建渐变色的矩形
    const size = { width: 15, height: 380 };
    g.append('rect')
        .attr('width', size.width)
        .attr('height', size.height)
        .attr('x', 5)
        .attr('y', 10)
        .style('fill', 'url(#gradient)');

    // 添加渐变色的文本标签
    g.append('text')
        .attr('x', 0)
        .attr('y', 0)
        .text(title)
        .style('font-size', '12px')
        .style('font-family', fontFamily)
        .style('fill', COLOR.label);

    // 添加渐变色的颜色标签
    g.selectAll('text.stop')
        .data([0, 20, 40, 60, 80, 100])
        .enter()
        .append('text')
        .attr('x', 30)
        .attr('y', d => Math.ceil((100 - d) / 100 * size.height) + 15)
        .text(d => `${d}%`)
        .style('font-size', '12px')
        .attr('class', 'stop')
        .style('fill', COLOR.label);
};
export const clear = (svg: d3.Selection<d3.BaseType, unknown, HTMLElement, any>): void => {
    svg.selectAll('*').remove();
};
export const drawGraph = (svg: d3.Selection<d3.BaseType, unknown, HTMLElement, any>, config: Igraph, theme: string, tDetails: TFunction): void => {
    clear(svg);
    COLOR = colorConfig[theme];
    addLegend(svg, `${tDetails('Peak')}(%)`);
    const graph = getDrawConfig(config, tDetails);
    const nodes = drawNode(svg, graph);
    drawContainer(nodes);
    drawRect(nodes);
    drawLine(nodes);
    addMarker(svg);
};

export const drawFlowChart = (svg: d3.Selection<d3.BaseType, unknown, HTMLElement, any>, data: ImemoryData & Icondition & {theme: string},
    tDetails: TFunction): void => {
    const graph: Igraph = getGraph(data);
    drawGraph(svg, graph, data.theme ?? 'dark', tDetails);
    updateData(svg, data);
};

export const getGraph = (data: ImemoryData): Igraph => {
    const { blockType = '', chipType = '', memoryUnit } = data;
    if (chipType.includes('910')) {
        // cube核有2种线路
        // 第1版：包含 GM_OR_L1_TO_L0A , GM_OR_L1_TO_L0B
        // 第2版：上面的2条分为 GM_TO_L0A_AIC，L1_TO_L0A_AIC，GM_TO_L0B_AIC，L1_TO_L0B_AIC
        const isVersion2 = ['cube', 'mix'].includes(blockType) &&
            memoryUnit.some(unit => String(unit.compare.memoryPath) === String(Path.GM_TO_L0A_AIC));
        return flow[`${blockType}910${isVersion2 ? 'V2' : ''}`] ?? [];
    } else if (chipType.includes('310')) {
        return flow.mix310;
    } else {
        return [];
    }
};

export const updateData = (svg: d3.Selection<d3.BaseType, unknown, HTMLElement, any>, data: ImemoryData & Icondition): void => {
    updateHitRatio(svg, data);
    updatePath(svg, data);
};

const getHitRatioData = (data: ImemoryData & Icondition): Record<string, CompareData<string | number>> => {
    return {
        hitRatio: {
            compare: getFormatNum(data?.l2Cache?.compare.hitRatio),
            diff: getFormatNumReturnEmpty(data?.l2Cache?.diff.hitRatio),
            baseline: getFormatNumReturnEmpty(data?.l2Cache?.baseline.hitRatio),
        } as CompareData<string | number>,
        vectorRatio: {
            compare: getFormatNum(data?.vector?.compare.ratio),
            diff: getFormatNumReturnEmpty(data?.vector?.diff.ratio),
            baseline: getFormatNumReturnEmpty(data?.vector?.baseline.ratio),
        } as CompareData<string | number>,
        vector1Ratio: {
            compare: getFormatNum(data?.vector1?.compare.ratio),
            diff: getFormatNumReturnEmpty(data?.vector1?.diff.ratio),
            baseline: getFormatNumReturnEmpty(data?.vector1?.baseline.ratio),
        } as CompareData<string | number>,
        cubeRatio: {
            compare: getFormatNum(data?.cube?.compare.ratio),
            diff: getFormatNumReturnEmpty(data?.cube?.diff.ratio),
            baseline: getFormatNumReturnEmpty(data?.cube?.baseline.ratio),
        } as CompareData<string | number>,
    };
};

const updateHitRatio = (svg: d3.Selection<d3.BaseType, unknown, HTMLElement, any>, data: ImemoryData & Icondition): void => {
    const dic = getHitRatioData(data);
    svg.selectAll('text.rect-labels')
        .text((d: any) => {
            let context: string = '';
            Object.keys(dic).forEach(item => {
                if (item === d.id) {
                    context = `${dic[d.id].compare}%`;
                }
                const baselineItem = `${item}Baseline`;
                if (baselineItem === d.id && dic[item].baseline !== '') {
                    context = `(${dic[item].baseline}%)`;
                }
            });
            if (context !== '') {
                return context;
            }
            return d.value ?? '';
        });
};

const getMemoryUnitLabel = (data: CompareData<ImemoryUnit>, isCompared: boolean, showAs: string): string => {
    let label = String(getFormatNum(data.compare[showAs]));
    if (showAs === 'bandwidth') {
        label = `${label} GB/s`;
    }
    if (isCompared) {
        const diffValue = data.baseline[showAs] === '' ? '-' : String(getFormatNum(data.baseline[showAs]));
        label = `${label}(${diffValue})`;
    }
    return label;
};

const updatePath = (svg: d3.Selection<d3.BaseType, unknown, HTMLElement, any>, data: ImemoryData & Icondition): void => {
    const { showAs, memoryUnit } = data;
    const labelDic: Record<string, string> = {};
    const peakDic: Record<string, number> = {};
    const peakSet = new Set<number>();
    memoryUnit.forEach(unit => {
        labelDic[unit.compare.memoryPath] = getMemoryUnitLabel(unit, data.isCompared, showAs);
        const peak = Number(unit.compare.peakRatio);
        if (!isNaN(peak)) {
            peakDic[unit.compare.memoryPath] = peak;
            peakSet.add(peak);
        }
    });
    updateLabel(svg, labelDic);
    updateHotPath(svg, peakDic, peakSet);
};
const updateLabel = (svg: d3.Selection<d3.BaseType, unknown, HTMLElement, any>, labelDic: Record<string, string>): void => {
    svg.selectAll('text.line-label')
        .text(d => {
            const lineid: any = ((d as IdrawLine)?.id ?? '');
            const memoryUnitId = Path[lineid];
            return labelDic[memoryUnitId] ?? '';
        });
};

const updateHotPath = (svg: d3.Selection<d3.BaseType, unknown, HTMLElement, any>, peakDic: Record<string, number>, peakSet: Set<number>): void => {
    const colorDic: Record<number, string> = {};
    [...peakSet].forEach(peakRatio => {
        colorDic[peakRatio] = computeColor(peakRatio);
    });
    const defs = svg.selectAll('defs').append('defs');
    const arrowMarker = defs.selectAll('marker')
        .data([...peakSet])
        .enter()
        .append('marker')
        .attr('id', d => `arrow${d}`)
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
        .attr('fill', d => colorDic[d]);
    svg.selectAll('polyline')
        .attr('stroke', d => {
            const lineid: any = ((d as IdrawLine)?.id ?? '').split('_append')[0];
            const memoryUnitId = Path[lineid];
            const peakRatio = peakDic[memoryUnitId];
            if (peakRatio !== undefined) {
                return colorDic[peakRatio];
            }
            return COLOR.line;
        })
        .attr('marker-end', d => {
            const lineid: any = ((d as IdrawLine)?.id ?? '').split('_append')[0];
            const memoryUnitId = Path[lineid];
            const peakRatio = peakDic[memoryUnitId];
            return `url(#arrow${peakRatio ?? ''})`;
        });
};
