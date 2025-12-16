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
                framework: res.framework || 'Unknown',
                algorithm: res.backendType || 'Unknown',
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
