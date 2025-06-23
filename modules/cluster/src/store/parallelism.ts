/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2025-2025. All rights reserved.
 */

import { makeAutoObservable } from 'mobx';
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

class ParallelismStore {
    generateConditions: GenerateConditions = defaultGenerateConditions;

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
            { key: 'ep-dp', tooltipKey: 'DPDimensionTooltip', label: 'DP' },
            { key: 'ep-dp-pp', tooltipKey: 'PPDimensionTooltip', label: 'DP + PP' },
            { key: 'ep-dp-pp-cp', tooltipKey: 'CPDimensionTooltip', label: 'DP + PP + CP' },
            {
                key: 'ep-dp-pp-cp-tp',
                tooltipKey: 'TPDimensionTooltip',
                label: this.generateConditions.cpSize === 1 ? 'DP + PP + TP' : 'DP + PP + CP + TP',
            },
        ];
        // 无 CP 维度时，选中 DP 维度
        if (this.generateConditions.cpSize === 1 && this.generateConditions.dimension === 'ep-dp-pp-cp') {
            this.updateGenerateConditions({ dimension: 'ep-dp' });
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

    expandDimension(activeRect: Rectangle | null): void {
        if (activeRect === null) {
            return;
        }

        const index = this.dimensionLevels.indexOf(this.activeDimension);
        if (index < this.dimensionLevels.length - 1) {
            this.activeDimension = this.dimensionLevels[index + 1];
        }
    }

    collapseDimension(activeRect: Rectangle | null): void {
        if (activeRect === null) {
            return;
        }

        const index = this.dimensionLevels.indexOf(this.activeDimension);
        if (index > 0) {
            this.activeDimension = this.dimensionLevels[index - 1];
        }
    }
}

const parallelismStore = new ParallelismStore();
export default parallelismStore;
