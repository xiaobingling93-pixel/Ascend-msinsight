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

import { ExpertLoadBalancingForm } from './Form';
import { ExpertLoadBalancingChart } from './Chart';
import React, { useCallback, useEffect, useState } from 'react';
import connector from '../../../connection';
import { importExpertData, queryModelInfo, queryExpertHotspot } from '../../../utils/RequestUtils';
import { QueryExpertHotspotItem } from '../../../utils/interface';
import { useRootStore } from '../../../context/context';
import { observer } from 'mobx-react';

export interface FormData {
    layerNum: number | null;
    denseLayerList: number[];
    expertNum: number | null;
    modelStage: 'prefill' | 'decode';
    version: 'unbalanced' | 'balanced' | 'profiling';
}

export const ExpertLoadBalancingBox = observer((): React.ReactElement => {
    const [formData, setFormData] = useState<FormData>({
        layerNum: null,
        denseLayerList: [],
        expertNum: null,
        modelStage: 'prefill',
        version: 'profiling',
    });
    const [chartData, setChartData] = useState<QueryExpertHotspotItem[]>([]);
    const [chartLoading, setChartLoading] = useState(false);
    const { sessionStore } = useRootStore();
    const session = sessionStore.activeSession;

    // 请求图表热点数据
    const fetchChatData = async (params?: FormData): Promise<void> => {
        const { layerNum, expertNum, modelStage, version, denseLayerList } = params ?? formData;

        try {
            if (layerNum === null || expertNum === null) {
                setChartData([]);
                return;
            }
            setChartLoading(true);
            const { hotspotInfos } = await queryExpertHotspot({
                layerNum,
                expertNum,
                modelStage,
                version,
                denseLayerList,
            });

            setChartLoading(false);
            setChartData(hotspotInfos);
        } catch (error) {
            setChartLoading(false);
        }
    };

    // 监听确认导入数据动作，执行导入数据逻辑
    const handleExpertDataImport = useCallback(async (e: MessageEvent<{ event: string; body: Record<string, unknown> }>): Promise<void> => {
        const res = e.data;
        if (res.body === undefined || typeof res.body !== 'object') {
            return;
        }

        const filePath = res.body?.path as string ?? '';
        if (filePath === '') {
            return;
        }

        await importExpertData({
            version: formData.version,
            filePath,
        });

        const conditions = await getFormConditions();
        await fetchChatData(conditions);
    }, [formData]);

    // 查询搜索条件
    const getFormConditions = async (): Promise<FormData> => {
        const data = await queryModelInfo();
        const processedData = {
            ...data,
            layerNum: data.layerNum === 0 ? null : data.layerNum,
            expertNum: data.expertNum === 0 ? null : data.expertNum,
        };
        const newFormData = { ...formData, ...processedData };
        setFormData(newFormData);
        return newFormData;
    };

    // 处理表单数据的变化
    const handleFormChange = (changedValues: any, newFormData: FormData): void => {
        setFormData(newFormData);
        if (changedValues.version !== undefined || changedValues.modelStage !== undefined) {
            fetchChatData(newFormData);
        }
    };

    const handleFormSubmit = (): void => {
        fetchChatData();
    };

    const initLoad = async (): Promise<void> => {
        const conditions = await getFormConditions();
        await fetchChatData(conditions);
    };

    const handleImport = (): void => {
        connector.send({
            event: 'openImportDialog',
            body: {},
        });
    };

    useEffect(() => {
        const { sequence } = connector.addListener('importExpertLoadDataConfirm', handleExpertDataImport);

        return () => {
            connector.removeListener({ event: 'importExpertLoadDataConfirm', sequence });
        };
    }, [handleExpertDataImport]);

    useEffect(() => {
        if (session.profilingExpertDataParsed && formData.version === 'profiling') {
            initLoad();
        }
    }, [session.profilingExpertDataParsed, session.renderId]);

    return <>
        <ExpertLoadBalancingForm
            loading={chartLoading}
            data={formData}
            onFormChange={handleFormChange}
            onFormSubmit={handleFormSubmit}
            onImport={handleImport}
        />
        <ExpertLoadBalancingChart
            loading={chartLoading}
            data={chartData}
        />
    </>;
});
