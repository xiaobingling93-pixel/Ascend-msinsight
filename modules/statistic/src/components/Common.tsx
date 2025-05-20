/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2025-2025. All rights reserved.
 */
import React from 'react';
export const Label = (props: {name: React.ReactNode;style?: object }): JSX.Element => {
    return <span style={{ marginRight: 8, ...(props.style ?? {}) }}>{props.name}{' :'} </span>;
};
