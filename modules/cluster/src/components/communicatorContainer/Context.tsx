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
