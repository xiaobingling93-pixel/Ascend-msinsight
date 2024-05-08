/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */
import React, { type CSSProperties } from 'react';

const Bar = ({ value = 0, max = 1 }: {value: number;max: number}): JSX.Element => {
    let label;
    if (isNaN(Number(value)) || isNaN(Number(max)) || Number(max) === 0) {
        label = <span style={{ marginLeft: '2px' }}>{value}</span>;
    } else {
        let width = Number(((value / max) * 100).toFixed(0));
        if (width < 0) {
            width = 0;
        }
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
        if (width >= 50) {
            label = (<div style={style} >
                <span style={{ marginRight: '2px', color: 'white' }}>{value}</span>
            </div>);
        } else {
            label = (<>
                <div style={style} ></div>
                <span style={{ marginLeft: '2px' }}>{value}</span>
            </>);
        }
    }
    return (<div style={{ minWidth: '100px' }} title={String(value)} className="bar">
        {label}
    </div>);
};
export default Bar;
