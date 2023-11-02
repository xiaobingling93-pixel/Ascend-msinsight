import { createContext, useContext } from 'react';
import { RootStore } from '../store';
import { RenderManager } from '../renderManager';
export const RootStoreContext = createContext<RootStore | undefined>(undefined);

export const useRootStore = (): RootStore => {
    const store = useContext(RootStoreContext);
    if (store === undefined) {
        throw new Error('RootStoreContext is undefined');
    }
    return store;
};

export const RenderManagerContext = createContext<RenderManager | undefined>(undefined);

export const useRenderManager = (): RenderManager => {
    const renderManager = useContext(RenderManagerContext);
    if (renderManager === undefined) {
        throw new Error('RenderManager is undefined');
    }
    return renderManager;
};
