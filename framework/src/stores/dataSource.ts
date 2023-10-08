import { ref, computed, watch, type Ref } from 'vue';
import { defineStore } from 'pinia';
import { type DataSource, LOCAL_HOST, PORT } from '@/centralServer/websocket/defs';
import type { TreeNodeType } from '@/components/MenuTree/types';
import { addDataPath, connectRemote, disconnectRemote, request } from '@/centralServer/server';
import connector from '@/connection';
import { modulesConfig } from '@/moduleConfig';
import { useSession } from './session';

const mergeDataSource = (dataSources: Ref<DataSource[]>, dataSource: DataSource): boolean => {
    const idx = dataSources.value.findIndex((item) => item.remote === dataSource.remote && item.port === dataSource.port);
    if (idx === -1) {
        dataSources.value.push(dataSource);
        return false;
    }
    // remove duplicated path
    dataSource.dataPath = dataSource.dataPath.filter(path => !dataSources.value[idx].dataPath.includes(path));
    if (dataSource.dataPath.length !== 0) {
        dataSources.value[idx].dataPath.push(...dataSource.dataPath);
        addDataPath(dataSource);
    }
    return true;
}

export const useDataSources = defineStore('dataSources', () => {
    const { session } = useSession();
    const dataSources = ref<DataSource[]>([{ remote: LOCAL_HOST, port: PORT, dataPath: [] }]);

    watch(session, () => {
        if (session.isReset) {
            dataSources.value.splice(1);
            dataSources.value[0].dataPath.splice(0, dataSources.value[0].dataPath.length - 1);
        }
    });

    const confirm = async (dataSource: DataSource): Promise<void> => {
        if (session.isReset) {
            session.reset();
            dataSources.value = [{ remote: LOCAL_HOST, port: PORT, dataPath: [] }];
        }
        const hasExistedServer = mergeDataSource(dataSources, dataSource);
        if (hasExistedServer) {
            return;
        }

        const isSuccess = await connectRemote(dataSource);
        isSuccess && connector.send({
            event: 'remote/import',
            body: { dataSource },
        });
    }

    const SPLITTER = ': ';
    const menuTree = computed<TreeNodeType[]>(() =>
        dataSources.value.filter(dataSource => dataSource.dataPath.length !== 0).map(dataSource => ({
            label: `${dataSource.remote}${SPLITTER}${dataSource.port}`,
            children: dataSource.dataPath.map(data => ({label: data })),
            cancelable: true,
        }))
    );

    const remove = async (index: number): Promise<void> => {
        const dataSource = dataSources.value[index];
        if (!dataSource) {
            return;
        }
        connector.send({
            event: 'remote/remove',
            body: { dataSource },
        });
        session.reset();
        if (dataSource.remote !== LOCAL_HOST) {
            dataSources.value.splice(index, 1);
        } else {
            dataSources.value[index].dataPath = [];
        }

        for (const module of modulesConfig) {
            // just request reset for displayed module
            if (module.isDefault || session.isCluster) {
                await request(dataSource, module.requestName, { command: 'remote/reset', params: {} });
            }
        }
        if (dataSource.remote !== LOCAL_HOST) {
            disconnectRemote(dataSource);
        }
    }

    const removeSingle = async (parentIndex: number, index: number): Promise<void> => {
        const dataSource = dataSources.value[parentIndex];
        if (!dataSource) {
            return;
        }
        const singleDataPath = dataSource.dataPath[index];
        connector.send({
            event: 'remote/removeSingle',
            body: { dataSource, singleDataPath },
        });
        dataSources.value[parentIndex].dataPath.splice(index, 1);
    }

    return { menuTree, remove, confirm, removeSingle };
});
