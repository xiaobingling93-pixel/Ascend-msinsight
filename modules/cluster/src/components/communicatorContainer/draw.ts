/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
*/
import type { partitionMode, ppData } from './ContainerUtils';
import * as d3 from 'd3';
import eventBus from '../../utils/eventBus';
import { cloneDeepWith } from 'lodash';

interface partitionModeItem {
    name: string;
    value: string;
    ranks: number[];
};
interface relatedRankType {
    [key: string]: any;
    pp: partitionModeItem[];
    tp: partitionModeItem[];
    dp: partitionModeItem[];
    dpRect: partitionModeItem[];
};

const rankSize = {
    width: 40,
    height: 46,
    padding: 4,
};
const ppContinerSize = {
    left: 28,
    top: 6,
    height: 80,
};
const dpContinerSize = {
    width: 0,
    height: 64,
    paddingTop: 18,
    paddingLeft: 8,
};
const tpContinerSize = {
    width: 0,
    height: 80,
    padding: 8,
};
let parallelSize = {
    ppSize: 0,
    dpSize: 0,
    tpSize: 0,
    preTpCount: 0,
};

const relatedRank: relatedRankType = {
    pp: [],
    tp: [],
    dp: [],
    dpRect: [],
};

const drawLineSVG = (svg: d3.Selection<d3.BaseType, unknown, HTMLElement, any>, data: ppData[], communicatorData: partitionMode[]): void => {
    if (data.length < 1) {
        return;
    }
    svg.selectAll('*').remove();
    parallelSize = {
        ppSize: data.length,
        dpSize: data[0].values.length,
        tpSize: data[0].values[0].values.length,
        preTpCount: data[0].values[0].values[0].values.length,
    };
    communicatorData.forEach(item => {
        relatedRank[item.mode] = cloneDeepWith(item.communicators);
    });
    drawHorizontalLine(svg);
    drawVerticalLine(svg);
    drawDpLine(svg);
    drawDpRect(svg);
    drawPpRect(svg);
    drawTpRect(svg);
};

const drawHorizontalLine = (svg: d3.Selection<d3.BaseType, unknown, HTMLElement, any>): void => {
    dpContinerSize.width = (dpContinerSize.paddingLeft * 2) +
        (parallelSize.tpSize * parallelSize.preTpCount * rankSize.width) +
        (parallelSize.tpSize * tpContinerSize.padding * 2);
    svg.attr('width', `${ppContinerSize.left + ((dpContinerSize.width + 4) * parallelSize.dpSize)}px`);
    if (parallelSize.dpSize < 2) {
        return;
    }
    let start = ppContinerSize.left + (rankSize.width / 2) + tpContinerSize.padding + 2;
    let end = start + dpContinerSize.width;
    const top = ppContinerSize.height + 2;
    let path = `${start},${top} ${start},${top - 30}`;
    const g = svg.append('g')
        .attr('class', 'horizontalLine')
        .attr('display', 'none');
    for (let i = 0; i < parallelSize.dpSize; i++) {
        g.append('polyline')
            .attr('points', path)
            .attr('stroke', '#6948C9')
            .attr('stroke-width', '2px')
            .attr('fill', 'none')
            .style('pointer-events', 'visibleStroke')
            .style('cursor', 'pointer');
        start = end;
        end = start + dpContinerSize.width;
        path = `${start},${top} ${start},${top - 30}`;
    }
    path = `${ppContinerSize.left + (rankSize.width / 2) + tpContinerSize.padding + 2},${top} ${start - dpContinerSize.width},${top}`;
    g.append('polyline')
        .attr('class', 'eventLine')
        .attr('points', path)
        .attr('stroke', '#6948C9')
        .attr('stroke-width', '2px')
        .attr('fill', 'none')
        .style('pointer-events', 'visibleStroke')
        .style('cursor', 'pointer');
};

const drawVerticalLine = (svg: d3.Selection<d3.BaseType, unknown, HTMLElement, any>): void => {
    svg.attr('height', `${(ppContinerSize.height * parallelSize.ppSize) + 5}px`);
    if (parallelSize.ppSize < 2) {
        return;
    }
    let start = ppContinerSize.top + rankSize.height;
    let end = start + ppContinerSize.height - (rankSize.height - 14);
    const left = ppContinerSize.left + (rankSize.width / 2) + 2;
    let path = `${left},${start} ${left},${end}`;
    const g = svg.append('g')
        .attr('class', 'verticalLine')
        .attr('display', 'none');
    for (let i = 1; i < parallelSize.ppSize; i++) {
        g.append('polyline')
            .attr('points', path)
            .attr('stroke', '#0277FF')
            .attr('stroke-width', '2px')
            .attr('fill', 'none')
            .style('pointer-events', 'visibleStroke')
            .style('cursor', 'pointer');
        start = start + ppContinerSize.height;
        end = end + ppContinerSize.height;
        path = `${left},${start} ${left},${end}`;
    }
};

const drawDpLine = (svg: d3.Selection<d3.BaseType, unknown, HTMLElement, any>): void => {
    if (parallelSize.preTpCount < 2) {
        return;
    }
    tpContinerSize.width = (parallelSize.preTpCount * rankSize.width) + (tpContinerSize.padding * 2);
    const width = rankSize.width - (rankSize.padding * 2);
    let startX = ppContinerSize.left + rankSize.width;
    const startY = ppContinerSize.top + 30;
    let end = startX + (rankSize.padding * 2);
    let path = `${startX},${startY} ${end},${startY}`;
    const g = svg.append('g')
        .attr('class', 'dpLine')
        .attr('display', 'none');
    for (let i = 1; i < parallelSize.preTpCount; i++) {
        g.append('polyline')
            .attr('points', path)
            .attr('stroke', '#09AF97')
            .attr('stroke-width', '2px')
            .attr('fill', 'none')
            .style('pointer-events', 'visibleStroke')
            .style('cursor', 'pointer');
        startX = end + width;
        end = startX + (rankSize.padding * 2);
        path = `${startX},${startY} ${end},${startY}`;
    }
};

const transformLine = (svg: d3.Selection<d3.BaseType, unknown, HTMLElement, any>,
    site: number[]): void => {
    const horizontalLineChangeX = (site[3] * rankSize.width) + (site[2] * tpContinerSize.padding * 2);
    const horizontalLineChangeY = site[0] * ppContinerSize.height;
    svg.select('.horizontalLine')
        .attr('display', '')
        .attr('transform', `translate(${horizontalLineChangeX},${horizontalLineChangeY})`)
        .selectAll('polyline')
        .on('click', () => {
            const index = (site[0] * parallelSize.tpSize * parallelSize.preTpCount) + site[3];
            const rankGroup = {
                name: `data${index}`,
                ranks: relatedRank.dp[index].ranks,
                value: relatedRank.dp[index].value,
            };
            eventBus.emit('activeCommunicator', rankGroup);
        });

    const verticalLineChangeX = (site[1] * dpContinerSize.width) + (site[3] * rankSize.width);
    svg.select('.verticalLine')
        .attr('display', '')
        .attr('transform', `translate(${verticalLineChangeX},0)`)
        .selectAll('polyline')
        .on('click', () => {
            const index = (site[1] * parallelSize.tpSize * parallelSize.preTpCount) + site[3];
            const rankGroup = {
                name: `pipeline${index}`,
                ranks: relatedRank.pp[index].ranks,
                value: relatedRank.pp[index].value,
            };
            eventBus.emit('activeCommunicator', rankGroup);
        });

    const dpLineChangeX = (site[1] * dpContinerSize.width) + (site[2] * tpContinerSize.width);
    const dpLineChangeY = site[0] * ppContinerSize.height;
    svg.select('.dpLine')
        .attr('display', '')
        .attr('transform', `translate(${dpLineChangeX},${dpLineChangeY})`)
        .selectAll('polyline')
        .on('click', () => {
            const index = (site[0] * parallelSize.dpSize) + site[1];
            const rankGroup = {
                name: `model${index}`,
                ranks: relatedRank.tp[index].ranks,
                value: relatedRank.tp[index].value,
            };
            eventBus.emit('activeCommunicator', rankGroup);
        });
};

const hideLine = (svg: d3.Selection<d3.BaseType, unknown, HTMLElement, any>): void => {
    svg.select('.horizontalLine')
        .attr('display', 'none');
    svg.select('.verticalLine')
        .attr('display', 'none');
    svg.select('.dpLine')
        .attr('display', 'none');
};

const drawDpRect = (svg: d3.Selection<d3.BaseType, unknown, HTMLElement, any>): void => {
    const g = svg.insert('g', '.horizontalLine')
        .attr('class', 'dpRect')
        .attr('display', 'none');
    let x = 21;
    const y = 4;
    const width = dpContinerSize.width - dpContinerSize.paddingLeft - 3;
    const height = (ppContinerSize.height * parallelSize.ppSize) - 8;
    for (let i = 0; i < parallelSize.dpSize; i++) {
        g.append('rect')
            .attr('x', x)
            .attr('y', y)
            .attr('width', width)
            .attr('height', height)
            .attr('rx', 5)
            .attr('ry', 5)
            .attr('stroke-width', '2px')
            .attr('stroke', '#6948C9')
            .attr('fill', 'none')
            .style('pointer-events', 'visibleStroke')
            .style('cursor', 'pointer')
            .on('click', () => {
                const rankGroup = {
                    name: `dataRect${i}`,
                    ranks: relatedRank.pp[i].ranks,
                    value: relatedRank.pp[i].value,
                };
                eventBus.emit('activeCommunicator', rankGroup);
            });
        x = x + dpContinerSize.width;
    }
};

const drawPpRect = (svg: d3.Selection<d3.BaseType, unknown, HTMLElement, any>): void => {
    const g = svg.insert('g', '.horizontalLine')
        .attr('class', 'ppRect')
        .attr('display', 'none');
    let x = 34;
    const y = 18;
    const width = rankSize.width - rankSize.padding;
    const height = (ppContinerSize.height * parallelSize.ppSize) - 30;
    for (let i = 0; i < parallelSize.dpSize; i++) {
        for (let j = 0; j < parallelSize.preTpCount; j++) {
            g.append('rect')
                .attr('x', x)
                .attr('y', y)
                .attr('width', width)
                .attr('height', height)
                .attr('rx', 5)
                .attr('ry', 5)
                .attr('stroke-width', '2px')
                .attr('stroke', '#0277FF')
                .attr('fill', 'none')
                .style('pointer-events', 'visibleStroke')
                .style('cursor', 'pointer')
                .on('click', () => {
                    const index = (i * parallelSize.tpSize * parallelSize.preTpCount) + j;
                    const rankGroup = {
                        name: `pipeline${index}`,
                        ranks: relatedRank.pp[index].ranks,
                        value: relatedRank.pp[index].value,
                    };
                    eventBus.emit('activeCommunicator', rankGroup);
                });
            x = x + width + rankSize.padding;
        }
        x = x + (dpContinerSize.paddingLeft * 2) + 16;
    }
};

const drawTpRect = (svg: d3.Selection<d3.BaseType, unknown, HTMLElement, any>): void => {
    const g = svg.insert('g', '.horizontalLine')
        .attr('class', 'tpRect')
        .attr('display', 'none');
    let x = 28;
    let y = 12;
    const drwaLine = (rect: d3.Selection<SVGGElement, unknown, HTMLElement, any>, indexI: number, indexJ: number): void => {
        rect.append('rect')
            .attr('x', x)
            .attr('y', y)
            .attr('width', width)
            .attr('height', height)
            .attr('rx', 5)
            .attr('ry', 5)
            .attr('stroke-width', '2px')
            .attr('stroke', '#09AF97')
            .attr('fill', 'none')
            .style('pointer-events', 'visibleStroke')
            .style('cursor', 'pointer')
            .on('click', () => {
                const index = (indexI * parallelSize.dpSize) + indexJ;
                const rankGroup = {
                    name: `model${index}`,
                    ranks: relatedRank.tp[index].ranks,
                    value: relatedRank.tp[index].value,
                };
                eventBus.emit('activeCommunicator', rankGroup);
            });
    };
    const width = (rankSize.width * parallelSize.preTpCount) + 7;
    const height = rankSize.height + 9;
    for (let i = 0; i < parallelSize.ppSize; i++) {
        for (let j = 0; j < parallelSize.dpSize; j++) {
            for (let k = 0; k < parallelSize.tpSize; k++) {
                drwaLine(g, i, j);
                x = x + tpContinerSize.width;
            }
            x = x + (dpContinerSize.paddingLeft * 2);
        }
        x = 28;
        y = y + ppContinerSize.height;
    }
};

const displayRect = (svg: d3.Selection<d3.BaseType, unknown, HTMLElement, any>, name: string, checked: boolean): void => {
    switch (name) {
        case 'Data Parallel':
            svg.select('.dpRect')
                .attr('display', checked ? '' : 'none');
            break;
        case 'Pipeline Parallel':
            svg.select('.ppRect')
                .attr('display', checked ? '' : 'none');
            break;
        case 'Tensor Parallel':
            svg.select('.tpRect')
                .attr('display', checked ? '' : 'none');
            break;
        default:
            break;
    }
};

export { drawLineSVG, transformLine, hideLine, displayRect };
