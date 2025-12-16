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
import { makeAutoObservable, observable } from 'mobx';

export interface OptionType { value: number | string; label: string };

// 筛选类型
export enum FilterType {
    MULTI_FILTER = 0,
    INPUT_FILTER = 1,
};

export class TabState {
    trigger?: { option1?: any; option2?: any };
    filter?: { type: FilterType; field: string; filterKeys: string[]; options: OptionType[] };
    data: any[] = [];
    search?: { fieldName: string; content?: string };
    options1?: OptionType[];
    options2?: OptionType[];
    switch?: { checked: boolean; tips: string; changeCallBack: (params: boolean) => void };

    constructor(conf?: Partial<TabState>) {
        makeAutoObservable(this, { data: observable.ref });
        if (conf) {
            if (conf.search !== undefined) {
                conf.search.content = '';
            }
            Object.assign(this, conf);
        }
    }

    getFilterData(): any[] {
        if (this.filter?.filterKeys !== undefined && this.filter.filterKeys.length > 0) {
            const data = this.data.filter((item: any) => this.filter?.filterKeys.includes(item[this.filter.field]));
            return data;
        }
        return this.data;
    }
}
