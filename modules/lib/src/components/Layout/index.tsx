/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */

import React, { type ReactNode } from 'react';

interface LayoutProps {
    children: ReactNode;
    padding?: number | string;
}
export const Layout: React.FC<LayoutProps> = ({ padding, children }): JSX.Element => {
    return (
        <div className="mi-page">
            <div className="mi-page-content" style={{ padding }}>
                {children}
            </div>
        </div>
    );
};
