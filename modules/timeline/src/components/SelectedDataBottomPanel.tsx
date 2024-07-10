import React, { useState } from 'react';
import { useTranslation } from 'react-i18next';
import styled from '@emotion/styled';
import { observer } from 'mobx-react';
import { SelectedDataBase } from './details/base/SelectedData';
import { Session } from '../entity/session';
import { SingleDataDesc } from '../entity/insight';
import { useSelectedDataDetailUpdater } from './details/hooks';
import { AscendSliceDetail } from '../entity/data';
import { ReactComponent as ExpandIcon } from '../assets/images/insights/PullDownIcon.svg';

interface DetailProps<T extends Record<string, unknown>> {
    session: Session;
    detail: SingleDataDesc<Record<string, unknown>, unknown>;
    children: React.FC<{data: T; session: Session}>;
};

const StyledSliceDetailDiv = styled.div`
    width: 100%;
    color: ${(props): string => props.theme.fontColor};
    display: flex;
    font-size: 12px;
`;

const StyledSliceArgsDiv = styled.div`
    width: 100%;
    color: ${(props): string => props.theme.fontColor};
    text-align: left;
    font-size: 12px;
`;

const createContentWithBreaks = (content: string): React.ReactNode => {
    return content?.split('\n').map((line, index) => (
        <React.Fragment key={index}>
            {line}
            <br />
        </React.Fragment>
    ));
};

const ArgsData = observer(({ data }: { data: AscendSliceDetail}): JSX.Element => {
    const argsJson = data.args;
    const [isHiddenArgs, setHidden] = useState(false);
    const { t } = useTranslation('timeline', { keyPrefix: 'sliceDetail' });
    if (argsJson === undefined) {
        return <></>;
    }
    try {
        const args = JSON.parse(argsJson);
        return <div>
            <StyledSliceArgsDiv>
                <ExpandIcon
                    onClick={ (): void => setHidden(!isHiddenArgs) } style={{ margin: '-2px 0 0 8px', float: 'left', transform: `rotate(${!isHiddenArgs ? 0 : '-90deg'}) translate(${!isHiddenArgs ? '-2' : '1'}px, ${!isHiddenArgs ? '0' : '-2'}px)`, cursor: 'pointer' }}/>
                <div style={{ fontWeight: 'bold', margin: '8px 0 0 8px' }}>{t('Args')}</div>
                {!isHiddenArgs
                    ? Object.keys(args).map(key => {
                        return <div style={{ marginLeft: '24px', height: '32px', lineHeight: '32px' }} key={key}>
                            <div style={{ minWidth: '110px', width: '150px', float: 'left', display: 'flex', whiteSpace: 'nowrap' }}>{key}</div>
                            <div style={{ float: 'left' }}>
                                { key === 'Call stack' ? createContentWithBreaks(args[key]) : args[key] }
                            </div>
                        </div>;
                    })
                    : <></>}
            </StyledSliceArgsDiv>
        </div>;
    } catch (e) {
        console.info('parse args fail:', e);
        return <></>;
    }
});

export const SelectedDataBottomPanel = observer(<T extends Record<string, unknown>>(props: DetailProps<T>): JSX.Element => {
    const { session, detail } = props;
    const { renderFields, data } = useSelectedDataDetailUpdater(session, detail, session.selectedData);
    const sliceDetailData = data as AscendSliceDetail;
    return <div style={{ height: '100%', overflow: 'scroll' }}><StyledSliceDetailDiv>
        <SelectedDataBase renderer={renderFields}/>
        <div></div>
    </StyledSliceDetailDiv>
    {(data as AscendSliceDetail).args === undefined ? <></> : <ArgsData data={sliceDetailData}/>}
    </div>;
});
