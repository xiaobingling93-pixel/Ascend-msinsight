/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */

import * as MindStudio from './mindstudio';
import { DefaultMindStudio } from './defaultMindStudio';
import * as Private from './privateMindStudio';
import { each, map, toArray } from '@lumino/algorithm';
import { URLExt } from '@jupyterlab/coreutils';
import { ServerConnection } from '@jupyterlab/services';

/**
 * The static namespace for DefaultMindStudio
 */

/**
 * Start a new mindstudio.
 *
 * @param name
 * @param options - The mindstudio options to use.
 *
 * @returns A promise that resolves with the mindstudio instance.
 */
export function startNew(
    name: string,
    options: MindStudio.IOptions = {}
): Promise<MindStudio.IMindStudio> {
    const serverSettings =
        options.serverSettings || ServerConnection.makeSettings();
    const url = Private.getServiceUrl(serverSettings.baseUrl);
    const contentType = 'Content-Type';
    const header = new Headers({ [contentType]: 'application/json' });

    const data = JSON.stringify({
        name: name,
    });

    const init = { method: 'POST', headers: header, body: data };

    return ServerConnection.makeRequest(url, init, serverSettings)
        .then((response: Response) => {
            if (response.status !== 200) {
                throw new ServerConnection.ResponseError(response);
            }
            return response.json();
        })
        .then((model: MindStudio.IModel) => {
            return new DefaultMindStudio(model.name, {
                ...options,
                serverSettings,
            });
        });
}

export function getStaticConfig(
    settings?: ServerConnection.ISettings
): Promise<MindStudio.IStaticConfig> {
    const localSettings = settings || ServerConnection.makeSettings();
    const statisConfigUrl = Private.getMindStudioStaticConfigUrl(
        localSettings.baseUrl
    );
    return ServerConnection.makeRequest(statisConfigUrl, {}, localSettings)
        .then((response: Response) => {
            if (response.status !== 200) {
                throw new ServerConnection.ResponseError(response);
            }
            return response.json();
        })
        .then((data: MindStudio.IStaticConfig) => {
            return data;
        });
}

/**
 * List the running mindstudio.
 *
 * @param settings - The server settings to use.
 *
 * @returns A promise that resolves with the list of running mindstudio models.
 */
export function listRunning(
    settings?: ServerConnection.ISettings
): Promise<MindStudio.IModel[]> {
    const localSettings = settings || ServerConnection.makeSettings();
    const serviceUrl = Private.getServiceUrl(localSettings.baseUrl);
    const instanceUrl = Private.getMindStudioInstanceRootUrl(
        localSettings.baseUrl
    );
    return ServerConnection.makeRequest(serviceUrl, {}, localSettings)
        .then((response: Response) => {
            if (response.status !== 200) {
                throw new ServerConnection.ResponseError(response);
            }
            return response.json();
        })
        .then((data: MindStudio.IModel[]) => {
            if (!Array.isArray(data)) {
                throw new Error('Invalid mindstudio data');
            }
            // Update the local data store.
            const urls = toArray(
                map(data, (item: MindStudio.IModel) => {
                    return URLExt.join(instanceUrl, item.name);
                })
            );
            each(Object.keys(Private.running), (runningUrl: string) => {
                if (urls.indexOf(runningUrl) === -1) {
                    const mindstudio = Private.running[runningUrl];
                    mindstudio.dispose();
                }
            });
            return data;
        });
}

/**
 * Shut down a mindstudio by name.
 *
 * @param name - Then name of the target mindstudio.
 *
 * @param settings - The server settings to use.
 *
 * @returns A promise that resolves when the mindstudio is shut down.
 */
export function shutdown(
    name: string,
    settings?: ServerConnection.ISettings
): Promise<void> {
    const localSettings = settings || ServerConnection.makeSettings();
    const url = Private.getMindStudioUrl(localSettings.baseUrl, name);
    const init = { method: 'DELETE' };
    return ServerConnection.makeRequest(url, init, localSettings).then((response: Response) => {
        Private.killMindStudio(url);
    });
}

/**
 * Shut down all mindstudio.
 *
 * @param settings - The server settings to use.
 *
 * @returns A promise that resolves when all the mindstudio are shut down.
 */
export function shutdownAll(
    settings?: ServerConnection.ISettings
): Promise<void> {
    const localSettings = settings || ServerConnection.makeSettings();
    return listRunning(localSettings).then(running => {
        each(running, (s: MindStudio.IModel) => {
            shutdown(s.name, localSettings);
        });
    });
}

/**
 * According mindstudio's name to get mindstudio's url.
 */
export function getUrl(
    name: string,
    settings?: ServerConnection.ISettings
): Promise<string> {
    const localSettings = settings || ServerConnection.makeSettings();
    const iframConfigUrl = Private.getIFrameConfigUrl(localSettings.baseUrl);
    return ServerConnection.makeRequest(iframConfigUrl, {}, localSettings)
    .then((response: Response) => {
        if (response.status !== 200) {
            throw new ServerConnection.ResponseError(response);
        }
        return response.json();
    })
    .then((data: any) => {
        return Private.getMindStudioInstanceUrl(localSettings.baseUrl, name, data.proxy, data.port);
    });
}
