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
import { createContext, useContext } from 'react';
import type { RootStore } from '../store';
import type { RenderEngine } from '../renderEngine';
import { ActionManager } from '../actions/manager';
export const RootStoreContext = createContext<RootStore | undefined>(undefined);

export const useRootStore = (): RootStore => {
    const store = useContext(RootStoreContext);
    if (store === undefined) {
        throw new Error('RootStoreContext is undefined');
    }
    return store;
};

export const RenderEngineContext = createContext<RenderEngine | undefined>(undefined);

export const useRenderEngine = (): RenderEngine => {
    const renderEngine = useContext(RenderEngineContext);
    if (renderEngine === undefined) {
        throw new Error('RenderEngine is undefined');
    }
    return renderEngine;
};

export const ActionManagerContext = createContext<ActionManager | undefined>(undefined);

export const useActionManager = (): ActionManager => {
    const actionManager = useContext(ActionManagerContext);
    if (actionManager === undefined) {
        throw new Error('ActionManagerContext is undefined');
    }
    return actionManager;
};
