/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
*/
import {
    DataRequest,
    ModuleName,
    ConnectHost,
    GLOBAL_HOST,
    Project,
    FileOrDirectory,
    ImportResultBody,
    DataSource,
    LayerType, ImportTreeInfo,
} from './websocket/defs';
import { Connection, ErrorMsg } from './websocket/connection';
import connector from '@/connection';
import {
    importProject,
    ImportProjectParams,
} from '@/utils/Request';
import { ProjectAction } from '@/utils/enum';
import { updateRankMap } from '@/utils/Rank';
import { transformFile, updateProject } from '@/utils/Project';
import { closeLoading } from '@/utils/useLoading';
import { updateDataScene } from '@/components/TabPane/Index';
import { Session } from '@/entity/session';

export const CONNECTION_MAP: Map<string, Connection> = new Map();

const getConnectionMapKey = (host: ConnectHost): string => {
    return `${host?.remote}:${host?.port}`;
};

export const connectRemote = async function (host: ConnectHost): Promise<boolean> {
    const connection = new Connection(host);
    try {
        await connection.connect();
    } catch (e) {
        return false;
    }
    CONNECTION_MAP.set(getConnectionMapKey(host), connection);
    return true;
};

export const isExistedRemote = function(host: ConnectHost): boolean {
    return CONNECTION_MAP.has(getConnectionMapKey(host));
};

export const addDataPath = async function(project: Project, action: ProjectAction, isConflict: boolean, session: Session): Promise<boolean> {
    if (!isExistedRemote(GLOBAL_HOST)) {
        const connected = await connectRemote(GLOBAL_HOST);
        if (!connected) {
            return false;
        }
    }
    const connection = CONNECTION_MAP.get(getConnectionMapKey(GLOBAL_HOST));
    if (connection) {
        connector.send({
            event: 'remote/savePageSetting',
            body: {},
            target: 'plugin',
        });
        const params: ImportProjectParams = {
            projectName: project.projectName,
            path: project.projectPath,
            selectedFileType: project.selectedFileType,
            selectedFilePath: project.selectedFilePath,
            selectedRankId: project.selectedRankId,
            projectAction: action,
            isConflict,
        };
        const result = await importProject(params);
        if (!result || (result as ErrorMsg).error !== undefined) {
            connector.send({ event: 'remote/reset', body: {}, target: 'plugin' }); // 由于发送时页签应该只有 时间线、内存、算子 存在，所以这个事件只能被这三个页签收到
            return false;
        }
        // 时间线需要判断是整体添加还是在项目内添加
        const { activeDataSource } = session;
        const projectAddFile = params?.projectAction !== ProjectAction.ADD_FILE || activeDataSource?.projectName !== params?.projectName;

        // 添加参数告诉是否为点击的添加
        connector.send({
            event: 'remote/import',
            body: { dataSource: transformTimelineDataSource(project), importResult: result as ImportResultBody, projectAddFile: projectAddFile },
            target: 'plugin',
        }); // 由于发送时页签应该只有 时间线、内存、算子 存在，所以这个事件只能被这三个页签收到
        afterImportProject(params, result as ImportResultBody);
    }
    return true;
};

const transformTimelineDataSource = (project: Project): DataSource => {
    const data = { ...GLOBAL_HOST, ...project, dataPath: project.projectPath };
    return data as DataSource;
};

/**
 * 递归查找 rank 列表
 * @param children
 */
const reduceImportRankInfoList = (children: ImportTreeInfo[] = []): Array<{ rankId: string; filePath: string }> => {
    const reduceFunc = (item: ImportTreeInfo, depth: number): Array<{ rankId: string; filePath: string }> => {
        if (depth >= 5) { // 设置最大递归深度为 5
            return [];
        }
        if (item.type === 'RANK' || item.type === 'COMPUTE' || item.type === 'IPYNB') {
            return [{ rankId: item.rankId ?? '', filePath: item.filePath }];
        } else {
            return item.children.flatMap((child) => reduceFunc(child, depth + 1));
        }
    };

    return children.flatMap((child) => reduceFunc(child, 0));
};

/**
 * 导入项目后更新项目管理器操作
 * @param params 请求体
 * @param data 返回值
 */
const afterImportProject = (params: ImportProjectParams, data: ImportResultBody): void => {
    try {
        const projectAction = params.projectAction as ProjectAction;
        const hasConflict = params.isConflict as boolean;
        const projectName = params.projectName;
        const projectPath = params.path;
        const importRankList = reduceImportRankInfoList(data.children);
        const selectedFilePath = params.selectedFilePath ?? importRankList?.[0]?.filePath ?? '';
        const selectedRankId = params.selectedRankId ?? importRankList?.[0]?.rankId ?? '';
        const selectedFileType: LayerType = params.selectedFileType ?? 'RANK'; // 此处暂时用 RANK，实际应该和 data.subdirectoryList[0] 中应该带有的类型相同
        const children = data.children?.map((child) => transformFile(child, 0))
            ?.flat()?.filter((item) => item !== undefined) as FileOrDirectory[];
        // 更新场景
        updateDataScene(data);
        // 更新rank信息
        updateRankMap(projectAction, projectName, importRankList);
        // 更新项目目录
        updateProject({ projectAction, projectName, children, hasConflict, projectPath, selectedFileType, selectedFilePath, selectedRankId });
    } catch (error) {
        console.error(error);
    }
    closeLoading();
};

export const request = function <T>(
    moduleName: ModuleName,
    args: DataRequest,
    voidResponse?: boolean,
): Promise<T | ErrorMsg> {
    const connection: Connection | undefined = CONNECTION_MAP.get(getConnectionMapKey(GLOBAL_HOST));
    if (connection === undefined) { return Promise.reject(Error('no connection')); }
    return new Promise((resolve: (v: T | ErrorMsg) => void, reject) => connection?.fetch<T>(moduleName, args, voidResponse)?.then(resolve, reject));
};
