/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */

import { useTheme } from '@emotion/react';
import styled from '@emotion/styled';
import { Checkbox, Tooltip } from 'ascend-components';
import type { CheckboxChangeEvent } from 'antd/lib/checkbox';
import { observer } from 'mobx-react';
import React, { Fragment, useState } from 'react';
import { ReactComponent as AntdFunnelIcon } from '../assets/images/insights/FunnelIcon.svg';
import type { InsightUnit } from '../entity/insight';
import type { Session } from '../entity/session';
import { CustomButton } from './base/StyledButton';
import type { SvgType } from './base/rc-table/types';

const funnelIcon = AntdFunnelIcon as SvgType;

// index is availableUnits`s index
const onChange = (session: Session, index: number, e: CheckboxChangeEvent): void => {
    // delete unit, find delete index in units.
    if (!e.target.checked) {
        const name = session.availableUnits[index].constructor;
        const insertIndex = getIndexInUnits(session, name);
        session.units = session.units.slice(0, insertIndex).concat(session.units.slice(insertIndex + 1));
        return;
    }
    // add unit, find add index in units.
    let j = 0;
    for (let i = 0; i < session.availableUnits.length && j < session.units.length;) {
        if (session.availableUnits[i].constructor !== session.units[j].constructor) {
            i++;
        } else {
            i++;
            j++;
        }
        if (index <= i) {
            break;
        }
    }
    const newSession: InsightUnit[] = session.units.slice(0, j);
    newSession.push(session.availableUnits[index]);
    session.units = newSession.concat(session.units.slice(j));
};

const getIndexInUnits = (session: Session, name: any): number => {
    for (let i = 0; i < session.units.length; i++) {
        if (session.units[i].constructor === name) {
            return i;
        }
    }
    return -1;
};

const CustomDiv = styled.div`
    border-radius: 18px;
    padding: 8px;
    background: ${(props): string => props.theme.tooltipBGColor};
`;

const UnitListContent = (session: Session): JSX.Element => {
    const theme = useTheme();

    return (
        <CustomDiv theme={theme}>
            {
                session.availableUnits.map((unit: InsightUnit, index: number) => {
                    return (
                        <Fragment key={unit.name}>
                            <Checkbox
                                style={{ color: theme.tooltipFontColor, backgroundColor: theme.tooltipBGColor }}
                                defaultChecked={getIndexInUnits(session, unit.constructor) !== -1}
                                onChange={(e: CheckboxChangeEvent): void => onChange(session, index, e)}>{unit.name}
                            </Checkbox>
                            <br />
                        </Fragment>
                    );
                })
            }
        </CustomDiv>
    );
};

export const SetUnits = observer(({ session }: { session: Session }): JSX.Element | null => {
    const theme = useTheme();
    const [isSuspend, updateIsSuspend] = useState(false);
    const onToolTipVisibleChange = (open: boolean): void => {
        updateIsSuspend(open);
    };
    return (
        <Tooltip title={UnitListContent(session)}
            key={session.id}
            trigger="click"
            color={theme.tooltipBGColor}
            overlayInnerStyle={{ color: theme.tooltipFontColor }}
            onOpenChange={onToolTipVisibleChange}
            placement="bottomLeft"
            align={{ offset: [0, 3] }}>
            <CustomButton icon={funnelIcon} isDisabled={session.phase !== 'configuring'} isSuspend={isSuspend} disabled={session.phase !== 'configuring'} />
        </Tooltip>
    );
});
