/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2025-2025. All rights reserved.
 */

import { makeAutoObservable } from 'mobx';
import { RootStore } from './RootStore';
import { TraceDataType, GetTraceDataResults } from '@/api/types';
import { getTraceData } from '@/api';
import { message } from 'antd';

export class TraceStore {
    rootStore: RootStore;
    loading = false;
    formData: TraceDataType = {
        framework: '-',
        algorithm: '-',
    };

    traceData: GetTraceDataResults | null = null;
    stageTypeList: string[] = [];

    constructor(rootStore: RootStore) {
        this.rootStore = rootStore;
        makeAutoObservable(this);
    }

    setLoading(val: boolean): void {
        this.loading = val;
    }

    getTraceData = async (): Promise<void> => {
        this.setLoading(true);

        try {
            const res = await getTraceData();
            this.traceData = res;
            this.stageTypeList = res.stageTypeList;
            this.formData = {
                framework: res.framework ?? 'Unknown',
                algorithm: res.backendType ?? 'Unknown',
            };
        } catch (error) {
            message.error('请求失败，请稍后重试');
        } finally {
            this.setLoading(false);
        }
    };

    reset(): void {
        this.loading = false;
        this.traceData = null;
        this.stageTypeList = [];
    }
}
