import { store, RootStore } from './rootStore';

export {
    store,
    RootStore,
};

window.debugInspector = { ...window.debugInspector, store };
