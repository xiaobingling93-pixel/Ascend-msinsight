/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 */
import { createContext, useContext } from 'react';
import { RootStore } from '../store';
export const RootStoreContext = createContext<RootStore | undefined>(undefined);

export const useRootStore = (): RootStore => {
    const store = useContext(RootStoreContext);
    if (store === undefined) {
        throw new Error('RootStoreContext is undefined');
    }
    return store;
};
