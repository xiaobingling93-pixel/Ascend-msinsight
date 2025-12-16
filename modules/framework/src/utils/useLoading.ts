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
import { store } from '@/store';
import { runInAction } from 'mobx';

// 整个页面的loading遮盖
const open = (): void => {
    const session = store.sessionStore.activeSession;
    runInAction(() => {
        session.loading = true;
    });
};

const close = (): void => {
    const session = store.sessionStore.activeSession;
    runInAction(() => {
        session.loading = false;
    });
};

function status(): boolean {
    const session = store.sessionStore.activeSession;

    return session.loading;
}

export { open as openLoading, close as closeLoading, status as loadingStatus };
