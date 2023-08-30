/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 */

import { ErrorBoundaryBase, UncaughtError } from './ErrorBoundaryBase';
import { Logger } from '../../utils/Logger';
import { Session } from '../../entity/session';
import i18n from '../../i18n';
import React from 'react';

export class AppErrorBoundary extends ErrorBoundaryBase {
    handleError = (error: UncaughtError): void => {
        // 根据不同子组件，按需设置session状态为error、右下角弹出提示、自定义状态/重置数据尝试恢复现场等等
        Logger('insightError', `${error.stack}`, 'error');
    };

    fallBackUI = (session: Session): JSX.Element => <div style={{
        display: 'flex',
        justifyContent: 'center',
        alignItems: 'center',
        width: '100%',
        height: '100%',
    }}><p>{i18n.t('error:4004')}</p></div>;
}
