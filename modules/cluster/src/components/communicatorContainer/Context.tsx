/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */

import React, { createContext, useContext, useState } from 'react';
import { ParallelismType } from '../../utils/interface';

interface ParallelSwitchConditionsContextType {
    dyeingMode: string;
    dyeingStep: number;
    parallelTypeList: ParallelismType[];
    setDyeingMode: (value: string) => void;
    setDyeingStep: (value: number) => void;
    setParallelTypeList: (values: ParallelismType[]) => void;
    reset: () => void;
}
const ParallelSwitchConditionsContext = createContext<ParallelSwitchConditionsContextType | undefined>(undefined);
export function ParallelSwitchConditionsProvider({ children }: { children: React.ReactNode }): JSX.Element {
    const [dyeingMode, setDyeingMode] = useState('None');
    const [dyeingStep, setDyeingStep] = useState(0.03);
    const [parallelTypeList, setParallelTypeList] = useState<ParallelismType[]>(['dp']);

    const reset = (): void => {
        setDyeingMode('None');
        setDyeingStep(0.03);
        setParallelTypeList(['dp']);
    };
    return (
        <ParallelSwitchConditionsContext.Provider value={{
            dyeingMode,
            setDyeingMode,
            dyeingStep,
            setDyeingStep,
            parallelTypeList,
            setParallelTypeList,
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
