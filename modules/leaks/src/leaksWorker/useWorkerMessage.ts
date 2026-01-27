/*
 * -------------------------------------------------------------------------
 * This file is part of the MindStudio project.
 * Copyright (c) 2026 Huawei Technologies Co.,Ltd.
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

import { store } from '../store';
import { useEffect } from 'react';
import { BlockWorker } from './blockWorker/worker';
import { runInAction } from 'mobx';

const { sessionStore } = store;
const session = sessionStore.activeSession;
const useWorkerMessage = (): void => {
    useEffect(() => {
        BlockWorker.onmessage = (ev: MessageEvent<any>): void => {
            if (session === undefined) {
                return;
            }
            switch (ev.data.type) {
                case 'dataInfo':
                    runInAction(() => {
                        session.leaksWorkerInfo.sizeInfo = ev.data.sizeInfo;
                        session.leaksWorkerInfo.renderOptions.zoom = ev.data.zoom;
                    });
                    break;
                case 'hoverItemResult':
                    runInAction(() => {
                        session.leaksWorkerInfo.hoverItem = ev.data.result;
                    });
                    break;
                case 'clickItemResult':
                    session.leaksWorkerInfo.clickItem = ev.data.result;
                    break;
                case 'print':
                    console.log(ev.data);
                    break;
                default:
                    break;
            }
        };
    }, []);
};

export default useWorkerMessage;
