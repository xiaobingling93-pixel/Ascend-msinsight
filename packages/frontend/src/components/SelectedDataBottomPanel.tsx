import React from 'react';
import styled from '@emotion/styled';
import { observer } from 'mobx-react';
import { SelectedDataBase } from './details/base/SelectedData';
import { Session } from '../entity/session';
import { SingleDataDesc } from '../entity/insight';
import { useSelectedDataDetailUpdater } from './details/hooks';
import { AscendSliceDetail } from '../entity/data';

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

const StyledSliceArgsDiv = styled.div`
    width: 100%;
    color: ${props => props.theme.fontColor};
    text-align: left;
    font-size: 12px;
`;

const ArgsData = observer(({ data }: { data: AscendSliceDetail}): JSX.Element => {
    const argsJson = data.args;
    if (argsJson === undefined) {
        return <></>;
    }
    try {
        const args = JSON.parse(argsJson);
        return <div>
            <StyledSliceArgsDiv>
                <div style={{ fontWeight: 'bold' }}>Args</div>
                {Object.keys(args).map(key => {
                    return <div style={{ marginLeft: '24px', height: '32px', lineHeight: '32px' }} key={key}><div style={{ minWidth: '110px', width: '10%', float: 'left', display: 'flex' }}>{key}</div>
                        <div style={{ width: '90%', float: 'left' }} dangerouslySetInnerHTML={{ __html: key === 'Call stack' ? args[key].replace(/\n/g, '<br>') : args[key] }}></div></div>;
                })}
            </StyledSliceArgsDiv>
        </div>;
    } catch (e) {
        console.info('parse args fail:', e);
        return <></>;
    }
});

export const SelectedDataBottomPanel = observer(function SelectedDataDetail<T extends Record<string, unknown>>(props: DetailProps<T>): JSX.Element {
    const { session, detail } = props;
    const { renderFields, data } = useSelectedDataDetailUpdater(session, detail, session.selectedData);
    const sliceDetailData = data as AscendSliceDetail;
    return <><StyledSliceDetailDiv>
        <SelectedDataBase renderer={renderFields}/>
        <div></div>
    </StyledSliceDetailDiv>
    {(data as AscendSliceDetail).args === undefined ? <></> : <ArgsData data={sliceDetailData}/>}
    </>;
});
