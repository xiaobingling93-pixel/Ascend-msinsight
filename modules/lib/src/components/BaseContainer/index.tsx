/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
*/
import React, { type CSSProperties, type ReactNode } from 'react';
import './index.css';

interface Iprops {
    header?: ReactNode;
    body?: ReactNode;
    style?: CSSProperties;
    headerStyle?: CSSProperties;
    bodyStyle?: CSSProperties;
    className?: string;
    headerClassName?: string;
    bodyClassName?: string;
}
function Index({ header, body, style, headerStyle, bodyStyle, className, headerClassName, bodyClassName }: Iprops): JSX.Element {
    return (<div className={`container-box ${className ?? ''}`} style={style ?? {}}>
        <div className={`container-header ${headerClassName ?? ''}`} style={headerStyle ?? {}}>
            {header}
        </div>
        <div className={`container-body ${bodyClassName ?? ''}`} style={bodyStyle ?? {} }>
            {body}
        </div>
    </div>);
};

export default Index;
