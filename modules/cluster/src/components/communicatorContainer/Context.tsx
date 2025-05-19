/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */

import React, { createContext, useContext, useState } from 'react';
import { ParallelismType } from '../../utils/interface';

interface ParallelSwitchConditionsContextType {
    dyeingMode: string;
    startVal: number | null;
    endVal: number | null;
    parallelTypeList: ParallelismType[];
    rankIndex: number | null;
    setDyeingMode: (value: string) => void;
    setStartVal: (value: number | null) => void;
    setEndVal: (value: number | null) => void;
    setParallelTypeList: (values: ParallelismType[]) => void;
    setRankIndex: (value: number | null) => void;
    reset: () => void;
}
const ParallelSwitchConditionsContext = createContext<ParallelSwitchConditionsContextType | undefined>(undefined);
export function ParallelSwitchConditionsProvider({ children }: { children: React.ReactNode }): JSX.Element {
    const [dyeingMode, setDyeingMode] = useState('None');
    const [startVal, setStartVal] = useState<ParallelSwitchConditionsContextType['startVal']>(null);
    const [endVal, setEndVal] = useState<ParallelSwitchConditionsContextType['endVal']>(null);
    const [parallelTypeList, setParallelTypeList] = useState<ParallelismType[]>(['dp']);
    const [rankIndex, setRankIndex] = useState<ParallelSwitchConditionsContextType['rankIndex']>(null);

    const reset = (): void => {
        setDyeingMode('None');
        setParallelTypeList(['dp']);
        setStartVal(null);
        setEndVal(null);
        setRankIndex(null);
    };
    return (
        <ParallelSwitchConditionsContext.Provider value={{
            dyeingMode,
            setDyeingMode,
            startVal,
            setStartVal,
            endVal,
            setEndVal,
            parallelTypeList,
            setParallelTypeList,
            rankIndex,
            setRankIndex,
            reset,
        }}>
            {children}
        </ParallelSwitchConditionsContext.Provider>
    );
}

export function useParallelSwitchConditions(): ParallelSwitchConditionsContextType {
    const context = useContext(ParallelSwitchConditionsContext);
    if (!context) {
        throw new Error('missing required parameters in ParallelSwitchConditionsProvider');
    }
    return context;
}
