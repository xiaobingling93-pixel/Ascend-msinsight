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
import { ErrorBoundaryBase, type UncaughtError } from './ErrorBoundaryBase';
import React from 'react';
import { logger } from '../../utils/Logger';
import { SessionPage } from '../../pages/SessionPage';
import type { Session } from '../../entity/session';

export class SessionPageErrorBoundary extends ErrorBoundaryBase {
    handleError = (error: UncaughtError): void => {
        // 根据不同子组件，按需设置session状态为error、右下角弹出提示、自定义状态/重置数据尝试恢复现场等等
        logger('insightError', `error in ${this.props.children.props.session.phase}@${this.props.children.props.session.id}：
        ${error.stack ?? ''}`, 'error');
    };

    fallBackUI = (session: Session): JSX.Element => {
        session.phase = 'error';
        this.reset();
        return <SessionPage session={session} />;
    };
}
