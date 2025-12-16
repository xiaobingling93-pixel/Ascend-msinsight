/*
 * -------------------------------------------------------------------------
 * This file is part of the MindStudio project.
 * Copyright (c) 2025 Huawei Technologies Co.,Ltd.
 *
 * MindStudio is licensed under Mulan PSL v2.
 * You can use this software according to the terms and conditions of the Mulan PSL v2.
 * You may obtain a copy of Mulan PSL v2 at:
 *
 *          http://license.coscl.org.cn/MulanPSL2
 *
 * THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
 * EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
 * MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
 * See the Mulan PSL v2 for more details.
 * -------------------------------------------------------------------------
 */

import React from 'react';
import styled from '@emotion/styled';
import { observer } from 'mobx-react';
import { useTranslation } from 'react-i18next';

interface timeDetailProps {
    renderer?: Array<[string, string | JSX.Element]>;
    hasTitle?: boolean;
}

const StyledSliceDetailDiv = styled.div`
    flex: 1;
    color: ${(props): string => props.theme.tableTextColor};
    font-size: 12px;

    .sliceDetailTitle {
        text-align: start;
        font-weight: bold;
        background: ${(props): string => props.theme.contentBackgroundColor};
    }
    .sliceDetail {
        display: flex;
        text-align: left;
        line-height: 32px;
        .sliceDetailName {
            flex: 1;
            padding-left: 24px;
            font-weight: bold;
        }
        .sliceDetailMsg {
            flex: 4;
            user-select: text;
            .iconContainer{
                display: flex;
                align-items: center;
                .jumpingIcon {
                    cursor: pointer;
                    margin: 2px 8px 0 5px;
                    g {
                        #Rectangle {
                            fill: ${(props): string => props.theme.svgPlayBackgroundColor};
                        }
                        path {
                            fill: ${(props): string => props.theme.selectedChartColor};
                        }
                    }
                }
            }
        }
    }
    .sliceDetail:hover {
        background-color: ${(props): string => props.theme.bgColorLight};
    }
`;

export const SelectedDataBase = observer((props: timeDetailProps): JSX.Element => {
    const { renderer, hasTitle = false } = props;
    const { t } = useTranslation('timeline', { keyPrefix: 'sliceDetail' });
    return <StyledSliceDetailDiv>
        {hasTitle && <div className = "sliceDetail">
            <div className = "sliceDetailName">{t('Event(s)')}</div>
            <div className = "sliceDetailMsg">{t('Link')}</div>
        </div>}
        {renderer?.map((item, index) => <div className = "sliceDetail" key={`${item[0]}-${index}`}>
            <div style={{ width: '30%' }} className = "sliceDetailName">{t(item[0], { defaultValue: item[0] })}</div>
            <div style={{ width: '70%' }} className = "sliceDetailMsg">{item[1]}</div>
        </div>)}
    </StyledSliceDetailDiv>;
});
