import React, { useEffect } from 'react';
import { observer } from 'mobx-react';
import { runInAction } from 'mobx';
import { useSelectedDataDetailUpdater } from './details/hooks';
import { SingleDataDesc } from '../entity/insight';
import { Session } from '../entity/session';
import styled from '@emotion/styled';
import { getDuration, getTimestamp } from '../utils/humanReadable';

const StyledSliceMoreDiv = styled.div`
    width: 100%;
    color: ${(props): string => props.theme.fontColor};
    display: flex;
    flex-direction: column;
    font-size: 12px;
    .sliceDetailKey {
        text-align: start;
        line-height: 32px;
        margin-left: 31px;
    }
`;

export const SelectedDetailKeyBottomPanel = observer(({ session, detail }: {session: Session; detail: SingleDataDesc<Record<string, unknown>, unknown>}): JSX.Element | null => {
    useEffect(() => {
        runInAction(() => {
            session.selectedDetailKeys = [];
            session.selectedDetails = [];
        });
    }, [session.selectedData]);
    const { data } = useSelectedDataDetailUpdater(session, detail, session.selectedDetailKeys);
    if (Object.keys(data).length !== 0) {
        return <StyledSliceMoreDiv>
            <div className="sliceDetailKey">
                <div>Expected Start Time: { getTimestamp(data.expectStartTime as number, { precision: 'ns' }) }</div>
                <div>Expected Duration: { getDuration(data.expectDuration as number, { precision: 'ns' }) }</div>
            </div>
        </StyledSliceMoreDiv>;
    }
    return <div className="empty">No Detail</div>;
}) as (props: { session: Session; detail: SingleDataDesc<Record<string, unknown>, unknown> }) => JSX.Element;
