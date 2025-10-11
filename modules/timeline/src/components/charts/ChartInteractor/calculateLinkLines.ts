/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2025-2025. All rights reserved.
 */
import { observable, observe } from 'mobx';
import * as d3 from 'd3';
import { getTimeOffset } from '../../../insight/units/utils';
import { Session } from '../../../entity/session';
import { getHeight, processIsCol, UNDRAW_HEIGHT } from './draw';
import { DataBlock, FlowEvent } from '../../FilterLinkLine';
import { handlerEmptyString } from '../../../utils/string';

function generateCalculateWHWithCache(): {
    calculateWHWithCacheFunc: Function;
    disposer: () => void; // 停止监听函数
} {
    const WIDTH_DEPENDENCIES = observable({
        canvasWith: 0,
        domainStart: 0,
        domainEnd: 0,
    });
    const WIDTH_CACHE: Map<string, number> = new Map();
    const HEIGHT_CACHE: Map<string, number | undefined> = new Map();
    const disposer = observe(WIDTH_DEPENDENCIES, (change) => {
        WIDTH_CACHE.clear(); // 根据宽度变化更新宽度缓存
    });
    function getWidthWithCache({ timestamp, cardId, pid }: { timestamp: number; cardId: string; pid: string },
        li: d3.ScaleLinear<number, number>, session: Session): number {
        const key = `${timestamp}-${cardId}-${pid}`;
        if (WIDTH_CACHE.has(key)) {
            return WIDTH_CACHE.get(key) ?? 0;
        }
        const width = li(timestamp - getTimeOffset(session, { cardId, processId: pid }));
        WIDTH_CACHE.set(key, width);
        return width;
    }

    function generateDataBlockKey(block: DataBlock): string {
        return `${block.pid}-${block.tid}-${block.depth}`;
    }

    function getHeightWithCache(block: DataBlock, cardId: string, category: string, session: Session): number | undefined {
        const key = `${generateDataBlockKey(block)}-${cardId}`;
        if (HEIGHT_CACHE.has(key)) {
            return HEIGHT_CACHE.get(key);
        }
        const height = getHeight(session, block, cardId, category);
        HEIGHT_CACHE.set(key, height);
        return height;
    }

    return {
        calculateWHWithCacheFunc: (canvasWith: number, domainStart: number, domainEnd: number) => {
            HEIGHT_CACHE.clear(); // 每次计算重置高度缓存
            WIDTH_DEPENDENCIES.canvasWith = canvasWith;
            WIDTH_DEPENDENCIES.domainStart = domainStart;
            WIDTH_DEPENDENCIES.domainEnd = domainEnd;
            return {
                getWidthWithCache,
                getHeightWithCache,
            };
        },
        disposer,
    };
}

const { calculateWHWithCacheFunc } = generateCalculateWHWithCache();

export interface LinkLineData {
    category: string;
    targetX: number;
    targetY: number;
    sourceX: number;
    sourceY: number;
    targetPos: Array<[x: number, y: number]>;
    offset: number;
}

export function calculateLinkLines(rawList: Array<Record<string, unknown>>, session: Session, ctx: CanvasRenderingContext2D): LinkLineData[] {
    const canvasWidth = ctx.canvas.clientWidth;
    const domainStart = session.domainRange.domainStart;
    const domainEnd = session.domainRange.domainEnd;
    const li = d3.scaleLinear().range([0, canvasWidth]).domain([domainStart, domainEnd]);
    const { getWidthWithCache, getHeightWithCache } = calculateWHWithCacheFunc(canvasWidth, domainStart, domainEnd);

    return rawList.map((data): LinkLineData => {
        const { category, from, to, cardId } = data as unknown as FlowEvent;
        const [targetCardId, sourceCardId] = [handlerEmptyString(to.rankId ?? '', cardId), handlerEmptyString(from.rankId ?? '', cardId)];

        const [targetX, targetY] = [getWidthWithCache({ timestamp: to.timestamp, cardId: targetCardId, pid: to.pid }, li, session),
            getHeightWithCache(to, targetCardId, category, session)];
        const [sourceX, sourceY] = [getWidthWithCache({ timestamp: from.timestamp, cardId: sourceCardId, pid: from.pid }, li, session),
            getHeightWithCache(from, sourceCardId, category, session)];
        const targetPos: Array<[x: number, y: number]> = [[targetX, targetY]];
        const offset = ((targetX - sourceX) / 2);
        const isAllCol = (processIsCol.get(`${targetCardId}-${to.pid}`) ?? false) &&
            (processIsCol.get(`${sourceCardId}-${from.pid}`) ?? false);
        return {
            category,
            targetX,
            targetY: isAllCol ? undefined : targetY,
            sourceX,
            sourceY: isAllCol ? undefined : sourceY,
            targetPos,
            offset,
        };
    }).filter(({ targetY, sourceY }) => !(sourceY === undefined || targetY === undefined) && !(sourceY < UNDRAW_HEIGHT && targetY < UNDRAW_HEIGHT));
}
