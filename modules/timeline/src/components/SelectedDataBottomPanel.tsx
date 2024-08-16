/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */

import React, { useState } from 'react';
import { useTranslation } from 'react-i18next';
import styled from '@emotion/styled';
import { observer } from 'mobx-react';
import { SelectedDataBase } from './details/base/SelectedData';
import type { Session } from '../entity/session';
import type { SingleDataDesc } from '../entity/insight';
import { useSelectedDataDetailUpdater } from './details/hooks';
import type { AscendSliceDetail } from '../entity/data';
import { ReactComponent as ExpandIcon } from '../assets/images/insights/PullDownIcon.svg';
import { Col, Row } from 'ascend-components';
import { customConsole as console } from 'ascend-utils';

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
        const breakKeys = ['Call stack', 'code'];
        return <div>
            <StyledSliceArgsDiv>
                <ExpandIcon
                    onClick={ (): void => setHidden(!isHiddenArgs) } style={{ margin: '-2px 0 0 8px', float: 'left', transform: `rotate(${!isHiddenArgs ? 0 : '-90deg'}) translate(${!isHiddenArgs ? '-2' : '1'}px, ${!isHiddenArgs ? '0' : '-2'}px)`, cursor: 'pointer' }}/>
                <div style={{ fontWeight: 'bold', margin: '8px 0 0 8px' }}>{t('Args')}</div>
                {!isHiddenArgs
                    ? Object.keys(args).map(key => {
                        return <Row key={key} style={{ marginLeft: '24px', lineHeight: '32px' }} >
                            <Col flex="150px" style={{ whiteSpace: 'nowrap' }}>{key}</Col>
                            <Col flex="auto" style={{ wordBreak: 'break-all' }}>
                                { breakKeys.includes(key) ? createContentWithBreaks(args[key]) : args[key] }
                            </Col>
                        </Row>;
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
