/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 */

import React, { ErrorInfo, ReactNode } from 'react';
import { Session } from '../../entity/session';
import { Logger } from '../../utils/Logger';

interface ErrorProps {
    children: JSX.Element;
}

interface ErrorState {
    hasError: boolean;
}

export type UncaughtError = Error;

export abstract class ErrorBoundaryBase<P = {}> extends React.Component<P & ErrorProps, ErrorState> {
    state = { hasError: false };

    componentDidCatch(error: Error, info: ErrorInfo): void {
        // 1、打印日志到底座log
        // 2、看DevEco有没有error report通道可以接入
        Logger('insightError', `componentStack:
        ${info.componentStack}`);
        this.handleError(error);
    }

    abstract handleError(error: UncaughtError): void;

    static getDerivedStateFromError(): {} {
        // 更新状态，使下一次渲染显示回退 UI
        return { hasError: true };
    }

    abstract fallBackUI(session: Session): JSX.Element;

    reset(): void {
        this.setState({ hasError: false });
    }

    render(): ReactNode {
        if (this.state.hasError) {
            return this.fallBackUI(this.props.children.props.session);
        }

        return this.props.children;
    }
}
