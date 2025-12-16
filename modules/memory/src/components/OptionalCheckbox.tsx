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
import { observer } from 'mobx-react-lite';
import { Checkbox } from '@insight/lib/components';
import { Label } from './Common';
import type { CheckboxChangeEvent } from 'antd/es/checkbox';

interface IProps {
    idKey: string;
    name: string;
    value: boolean;
    onChange: (v: CheckboxChangeEvent) => void;
    visible: boolean;
    disabled?: boolean;
}

const OptionalCheckbox = observer((props: IProps) => {
    const {
        idKey,
        name = '',
        value = false,
        onChange = (): void => {},
        visible = false,
        disabled = false,
    } = props;
    if (!visible) {
        return <></>;
    } else {
        return <div className="flex items-center">
            <Label name={name} />
            <Checkbox
                id={idKey}
                checked={value}
                onChange={onChange}
                disabled={disabled}
            />
        </div>;
    }
});

export default OptionalCheckbox;
