import { ref, computed, watch, type Ref } from 'vue';
import { defineStore } from 'pinia';
import { type DataSource, LOCAL_HOST, PORT } from '@/centralServer/websocket/defs';
import type { TreeNodeType } from '@/components/MenuTree/types';
import { addDataPath, connectRemote, disconnectRemote, request } from '@/centralServer/server';
import connector from '@/connection';
import { modulesConfig } from '@/moduleConfig';
import { Session, useSession } from './session';

export type FormItemData = { value: string; status: 'wait' | 'error' | 'success' };
type FormDataSource = {
    remote: FormItemData;
    port: FormItemData;
    dataPath: FormItemData[];
}

const validator = (formItemData: FormItemData, rule: RegExp | null): boolean => {
    if (!rule) { return true; }
    return rule.test(formItemData.value);
}

const validateAll = (formData: FormDataSource): boolean => {
    let rule: RegExp | null;
    let res = true;
    for (const key in formData) {
        if (key === 'remote') {
            rule = /^(\d{1,2}|1\d\d|2[0-4]\d|25[0-5])\.(\d{1,2}|1\d\d|2[0-4]\d|25[0-5])\.(\d{1,2}|1\d\d|2[0-4]\d|25[0-5])\.(\d{1,2}|1\d\d|2[0-4]\d|25[0-5])$/;
        } else if (key === 'port') {
            rule = /^([0-9]|[1-9]\d|[1-9]\d{2}|[1-9]\d{3}|[1-5]\d{4}|6[0-4]\d{3}|65[0-4]\d{2}|655[0-2]\d|6553[0-5])$/;
        } else {
            rule = null;
        }
        if (Array.isArray(formData[key as keyof FormDataSource])) { continue; }
        if (!validator((formData[key as keyof FormDataSource] as FormItemData), rule)) {
            (formData[key as keyof FormDataSource] as FormItemData).status = 'error';
            res = false;
        } else {
            (formData[key as keyof FormDataSource] as FormItemData).status = 'success';
        }
    }
    return res;
}

const useWatchReset = (session: Omit<Session, '_sharedState'>, dataSources: Ref<DataSource[]>): void => {
    watch(session, () => {
        if (session.isReset) {
            dataSources.value = [];
            session.reset();
        }
    });
};


const mergeDataSource = (dataSources: Ref<DataSource[]>, dataSource: DataSource): boolean => {
    const idx = dataSources.value.findIndex((item) => item.remote === dataSource.remote && item.port === dataSource.port);
    if (idx === -1) {
        dataSources.value.push(dataSource);
        return false;
    }
    dataSources.value[idx].dataPath.push(...dataSource.dataPath);
    addDataPath(dataSource);
    return true;
}

export const useDataSources = defineStore('dataSources', () => {
    const { session } = useSession();
    const dataSources = ref<DataSource[]>([{ remote: LOCAL_HOST, port: PORT, dataPath: [] }]);
    useWatchReset(session, dataSources);

    let temp: FormDataSource | null = null;
    const add = (originSource: FormDataSource) => {
        temp = originSource;
    }

    const confirm = async (): Promise<boolean> => {
        if (!temp || !validateAll(temp)) {
            return false;
        }

        const dataSource: DataSource = {
            remote: temp.remote.value,
            port: Number(temp.port.value || 0),
            dataPath: temp.dataPath.map(data => data.value),
        };

        const hasExistedServer = mergeDataSource(dataSources, dataSource);
        if (hasExistedServer) {
            return true;
        }

        const isSuccess = await connectRemote(dataSource);
        isSuccess && connector.send({
            body: {
                event: 'remote/import',
                body: { dataSource },
            }
        });
        return isSuccess;
    }

    const cancel = (): void => {
        temp = null;
    }

    const SPLITTER = ': ';
    const menuTree = computed<TreeNodeType[]>(() =>
        dataSources.value.filter(dataSource => dataSource.dataPath.length !== 0).map(dataSource => ({
            content: `${dataSource.remote}${SPLITTER}${dataSource.port}`,
            children: dataSource.dataPath.map(data => ({ content: data })),
            cancelable: true,
        }))
    );

    const remove = async (index: number): Promise<void> => {
        menuTree.value.splice(index, 1);
        connector.send({
            body: {
                event: 'remote/remove',
                body: { dataSource: dataSources.value[index] },
            }
        });
        for (const module of modulesConfig) {
            await request(dataSources.value[index], module.requestName, { command: 'remote/reset', params: {} });
        }
        disconnectRemote(dataSources.value[index]);
        dataSources.value.splice(index, 1);
    }

    return { menuTree, add, remove, confirm, cancel };
});
