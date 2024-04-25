/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
*/
import { Descriptions } from 'antd';
import React, { type ReactNode } from 'react';
import './BaseDescriptions.css';
interface Iitem {
    label: ReactNode;
    value: ReactNode;
    showColon?: boolean;
}
interface Iporps {
    items: Iitem[];
}
function BaseDescription({ items }: Iporps): JSX.Element {
    return (
        <Descriptions size={'small'} bordered className={'infocard'}>
            {
                items.map((item, index) => (
                    <Descriptions.Item
                        key={`${item.label}_${index}`}
                        label={<div>
                            {item.label}
                            <span style={{ display: item.showColon === false ? 'none' : 'inline-block' }}>&nbsp;:</span>
                        </div>}
                        span={3}
                    >
                        {item.value}
                    </Descriptions.Item>
                ))
            }
        </Descriptions>
    );
}

export default BaseDescription;
