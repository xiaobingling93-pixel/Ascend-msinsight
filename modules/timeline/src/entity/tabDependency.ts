import { makeAutoObservable, observable } from 'mobx';

export type OptionType = { value: number | string; label: string };

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
    switch?: { checked: boolean; tips: string; changeCallBack: Function };

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
