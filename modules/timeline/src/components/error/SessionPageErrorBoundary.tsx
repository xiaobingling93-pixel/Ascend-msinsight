/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 */
import { ErrorBoundaryBase, UncaughtError } from './ErrorBoundaryBase';
import React from 'react';
import { Logger } from '../../utils/Logger';
import { SessionPage } from '../../pages/SessionPage';
import { Session } from '../../entity/session';

export class SessionPageErrorBoundary extends ErrorBoundaryBase {
    handleError = (error: UncaughtError): void => {
        // 根据不同子组件，按需设置session状态为error、右下角弹出提示、自定义状态/重置数据尝试恢复现场等等
        Logger('insightError', `error in ${this.props.children.props.session.phase}@${this.props.children.props.session.id}：
        ${error.stack ?? ''}`, 'error');
    };

    fallBackUI = (session: Session): JSX.Element => {
        session.phase = 'error';
        this.reset();
        return <SessionPage session={session} />;
    };
}
