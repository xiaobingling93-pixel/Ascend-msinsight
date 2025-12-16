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
