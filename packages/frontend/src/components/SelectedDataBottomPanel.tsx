import React from 'react';
import styled from '@emotion/styled';
import { observer } from 'mobx-react';
import { SelectedDataBase } from './details/base/SelectedData';
import { Session } from '../entity/session';
import { SingleDataDesc } from '../entity/insight';
import { useSelectedDataDetailUpdater } from './details/hooks';

interface DetailProps<T extends Record<string, unknown>> {
    session: Session;
    detail: SingleDataDesc<Record<string, unknown>, unknown>;
    children: React.FC<{data: T; session: Session}>;
};

const StyledSliceDetailDiv = styled.div`
    width: 100%;
    color: ${props => props.theme.fontColor};
    display: flex;
    font-size: 12px;
`;

export const SelectedDataBottomPanel = observer(function SelectedDataDetail<T extends Record<string, unknown>>(props: DetailProps<T>): JSX.Element {
    const { session, detail } = props;
    const { renderFields, data } = useSelectedDataDetailUpdater(session, detail, session.selectedData);

    return <StyledSliceDetailDiv>
        <SelectedDataBase renderer={renderFields} />
        { props.children({ data: data as T, session }) }
    </StyledSliceDetailDiv>;
});
