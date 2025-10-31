/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */

import { observer } from 'mobx-react';
import { Session } from '../entity/session';
import { useRef, useState } from 'react';
import { CustomButton } from './base/StyledButton';
import { handleTimeMakerAction } from '../utils/TimeMakerUtils';
import * as React from 'react';
import styled from '@emotion/styled';
import { useWatchResize } from '../utils/useWatchDomResize';
import type { TFunction } from 'i18next';
import { useTranslation } from 'react-i18next';
import { ReactComponent as AntdPlaceFlagButtonSvg } from '../assets/images/timeline/ic_place_flag.svg';
import { FlagIcon } from 'ascend-icon';
import { addRangeFlag, deleteRangeFlag, linearScaleFactory, transformTimeToLeft } from './TimelineMarker';
import { runInAction } from 'mobx';
import { getTimestamp } from '../utils/humanReadable';
import type { SvgType } from './base/rc-table/types';
import { adaptDpr } from 'ascend-utils';

const PlaceFlagButtonSvg = AntdPlaceFlagButtonSvg as SvgType;

export const TimeMakerButton = observer(({ session }: { session: Session }): JSX.Element | null => {
    const { t } = useTranslation();
    const [isSuspend, updateIsSuspend] = useState(false);
    const onToolTipVisibleChange = (open: boolean): void => {
        updateIsSuspend(open);
    };
    const timeMakerProps = { session, onToolTipVisibleChange };
    return <CustomButton data-testid={'tool-marker'} icon={FlagIcon as any} tooltip={t('timelineMarker:markerList')}
        isSuspend={ isSuspend } onClick={ (): void => { handleTimeMakerAction(timeMakerProps); }}/>;
});

const CanvasContainer = styled.div`
    width: 100%;
`;

const rangeButtonCanvasId = 'rangeButtonCanvas';

interface RangeMarkerButtonProps {
    session: Session;
    timelineHeight: number;
}

export const RangeMarkerButtonCanvas = observer(({ session, timelineHeight }: RangeMarkerButtonProps): JSX.Element => {
    const { t } = useTranslation();
    const range = useRef<[ number, number ]>([0, 0]);
    const canvas = React.useRef<HTMLCanvasElement>(null);
    const { domainStart, domainEnd } = session.domainRange;
    const [width, ref] = useWatchResize<HTMLDivElement>('width');
    React.useEffect(() => {
        if (!canvas.current) {
            return () => {};
        }
        const singleClickListener = (e: MouseEvent): void => handleSingleClick(e, session, range, domainStart, domainEnd);
        if (session.name !== t('Realtime Monitor')) {
            addEventListener('click', singleClickListener);
        }
        drawPlaceRangeButton(session, canvas.current, [domainStart, domainEnd], t);
        range.current = [0, canvas.current.clientWidth];
        return () => {
            removeEventListener('click', singleClickListener);
        };
    }, [width, domainStart, domainEnd, session.timelineMaker.refreshTrigger, session.selectedRange]);

    React.useEffect(() => {
        const ctx = canvas.current?.getContext('2d');
        if (!canvas.current || !ctx) {
            return;
        }
        ctx.clearRect(0, 0, canvas.current.width, canvas.current.height);
    }, [session.sliceSelection.active]);

    return <CanvasContainer ref={ref}>
        <canvas
            id={ 'rangeButtonCanvas' }
            ref={canvas}
            width={ width }
            height={ timelineHeight }
            style={{ width, height: timelineHeight, position: 'absolute', top: 0, left: 0 }}/>
        <PlaceFlagButtonSvg style={{ display: 'none' }}/>
    </CanvasContainer>;
});

const drawPlaceRangeButton = (session: Session, current: HTMLCanvasElement, domain: number[], t: TFunction): void => {
    const ctx = current.getContext('2d');
    const selectedRange = session.selectedRange;
    if (!ctx || session.name === t('Realtime Monitor')) {
        return;
    }
    let buttonSvg: SVGSVGElement | undefined;
    document.querySelectorAll('svg').forEach(svgItem => {
        if (svgItem.id === 'ic_place_flag') { buttonSvg = svgItem; }
    });
    const { canvasWidth } = adaptDpr(current, ctx);
    ctx.clearRect(0, 0, current.width, current.height);
    if (buttonSvg === undefined || selectedRange === undefined) { return; }
    const rangeEndTimeStamp = selectedRange[0] > selectedRange[1] ? selectedRange[0] : selectedRange[1];
    const buttonWith = 18;
    const beginX = transformTimeToLeft(domain[0], domain[1], rangeEndTimeStamp, canvasWidth) - buttonWith;
    buttonSvg.style.fill = session.timelineMaker.oldMarkedRange === undefined ? '#5291FF' : '#5BA854';
    const buttonSvgString = new XMLSerializer().serializeToString(buttonSvg);
    const buttonSvgData = `data:image/svg+xml;base64,${btoa(buttonSvgString)}`;
    const buttonImage: HTMLImageElement = new Image();
    buttonImage.src = buttonSvgData;
    buttonImage.onload = (): void => {
        const buttonMarginTop = 10;
        ctx.drawImage(buttonImage, beginX, buttonMarginTop);
    };
};

const handleSingleClick = (e: MouseEvent, session: Session, range: React.MutableRefObject<[number, number]>, domainStart: number, domainEnd: number): void => {
    const rangeButtonCanvas = document.elementsFromPoint(e.clientX, e.clientY).find(t => t.id === rangeButtonCanvasId);
    if (!rangeButtonCanvas || session.selectedRange === undefined) {
        return;
    }
    const xScale = linearScaleFactory([domainStart, domainEnd], range.current);
    const buttonWith = 20;
    const rangeStartTimestamp = session.selectedRange[0] < session.selectedRange[1] ? session.selectedRange[0] : session.selectedRange[1];
    const rangeStartTimeDisplay = getTimestamp(rangeStartTimestamp, { precision: session.isNsMode ? 'ns' : 'ms' });
    const rangeEndTimestamp = session.selectedRange[0] > session.selectedRange[1] ? session.selectedRange[0] : session.selectedRange[1];
    const rangeEndOffset = Math.floor(xScale(rangeEndTimestamp));
    if (e.offsetX > rangeEndOffset || e.offsetX < rangeEndOffset - buttonWith) {
        return;
    }
    runInAction(() => {
        if (session.timelineMaker.oldMarkedRange === undefined) {
            session.timelineMaker.oldMarkedRange = session.selectedRange;
            addRangeFlag(session, rangeStartTimestamp, rangeStartTimeDisplay, rangeEndTimestamp);
        } else {
            session.timelineMaker.oldMarkedRange = undefined;
            deleteRangeFlag(session, rangeStartTimestamp, rangeEndTimestamp);
        }
    });
};
