/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */
import React, { type CSSProperties } from 'react';

const Bar = ({ value = 0, sum = 1 }: {value: number;sum: number}): JSX.Element => {
    const width = Number(((value / sum) * 100).toFixed(0));
    const style: CSSProperties = {
        background: 'var(--lightblue)',
        borderRadius: '1px',
        display: 'inline-block',
        width: `${width}%`,
        color: 'white',
        float: 'left',
        textAlign: 'right',
        overflow: 'hidden',
        height: '20px',

    };
    return (<div style={{ minWidth: '100px' }} title={String(value)} className="bar">
        {width >= 50
            ? (
                <div
                    style={style}
                ><span style={{ marginRight: '2px', color: 'white' }}>{value}</span></div>
            )
            : (
                <>
                    <div style={style} ></div>
                    <span style={{ marginLeft: '2px' }}>{value}</span>
                </>
            )
        }
    </div>);
};
export default Bar;
