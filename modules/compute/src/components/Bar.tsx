/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */
import React from 'react';

const Bar = ({ value = 0, sum = 1 }: {value: number;sum: number}): JSX.Element => {
    const width = Number(((value / sum) * 100).toFixed(0));
    const style: React.CSSProperties = {
        background: 'var(--lightblue)',
        border: '1px solid white',
        borderRadius: '1px',
        display: 'inline-block',
        width: `${width}%`,
        color: 'white',
        textAlign: 'right',
        overflow: 'hidden',
        height: '20px',

    };
    return (<div style={{ minWidth: '100px' }} title={String(value)}>
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
