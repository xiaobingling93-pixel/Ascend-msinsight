/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
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
}

const OptionalCheckbox = observer((props: IProps) => {
    const {
        idKey,
        name = '',
        value = false,
        onChange = (): void => {},
        visible = false,
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
            />
        </div>;
    }
});

export default OptionalCheckbox;
