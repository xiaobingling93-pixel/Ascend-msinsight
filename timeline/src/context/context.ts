import { createContext, useContext } from 'react';
import { RootStore } from '../store';
import { RenderEngine } from '../renderEngine';
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
