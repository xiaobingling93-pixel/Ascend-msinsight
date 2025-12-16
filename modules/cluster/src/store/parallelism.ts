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

import { configure, makeAutoObservable } from 'mobx';
import { ParallelismArrangementParams } from '../utils/interface';
import { Rectangle } from '../components/communicatorContainer/shape';

export type GenerateConditions = ParallelismArrangementParams;
export const defaultGenerateConditions: GenerateConditions = {
    algorithm: 'megatron-lm(tp-cp-ep-dp-pp)',
    dimension: 'ep-dp-pp',
    dpSize: 1,
    ppSize: 1,
    tpSize: 1,
    epSize: 1,
    cpSize: 1,
    moeTpSize: 1,
};

type Dimension = GenerateConditions['dimension'];
export interface DimensionOption {
    key: Dimension;
    tooltipKey: string;
    label: string;
}

configure({ enforceActions: 'always' });
class ParallelismStore {
    generateConditions: GenerateConditions = defaultGenerateConditions;
    rectToExpand: Rectangle | null = null;
    rectToCollapsed: Rectangle | null = null;

    constructor() {
        makeAutoObservable(this);
    }

    get activeDimension(): Dimension {
        return this.generateConditions.dimension;
    }

    set activeDimension(value: Dimension) {
        this.generateConditions.dimension = value;
    }

    get dimensionOptionsData (): DimensionOption[] {
        const optionsList: DimensionOption[] = [
            { key: 'ep-dp-pp', tooltipKey: 'PPDimensionTooltip', label: 'DP + PP' },
            { key: 'ep-dp-pp-cp', tooltipKey: 'CPDimensionTooltip', label: 'DP + PP + CP' },
            {
                key: 'ep-dp-pp-cp-tp',
                tooltipKey: 'TPDimensionTooltip',
                label: this.generateConditions.cpSize === 1 ? 'DP + PP + TP' : 'DP + PP + CP + TP',
            },
        ];
        // 无 CP 维度时，选中 默认维度
        if (this.generateConditions.cpSize === 1 && this.generateConditions.dimension === 'ep-dp-pp-cp') {
            this.updateGenerateConditions({ dimension: 'ep-dp-pp' });
        }

        return optionsList.filter(option => {
            // 当 cpSize = 1，隐藏 cp 维度视图
            return !(this.generateConditions.cpSize === 1 && option.key === 'ep-dp-pp-cp');
        });
    }

    get dimensionLevels(): Dimension[] {
        return this.dimensionOptionsData.map(item => item.key);
    }

    updateGenerateConditions(value: Partial<GenerateConditions>): void {
        Object.assign(this.generateConditions, value);
    }

    expandDimension(activeRect: Rectangle): void {
        this.rectToExpand = activeRect;

        const index = this.dimensionLevels.indexOf(this.activeDimension);
        if (index < this.dimensionLevels.length - 1) {
            this.activeDimension = this.dimensionLevels[index + 1];
        }
    }

    collapseDimension(activeRect: Rectangle): void {
        this.rectToCollapsed = activeRect;

        const index = this.dimensionLevels.indexOf(this.activeDimension);
        if (index > 0) {
            this.activeDimension = this.dimensionLevels[index - 1];
        }
    }

    reset(): void {
        this.generateConditions = defaultGenerateConditions;
    }
}

const parallelismStore = new ParallelismStore();
export default parallelismStore;
