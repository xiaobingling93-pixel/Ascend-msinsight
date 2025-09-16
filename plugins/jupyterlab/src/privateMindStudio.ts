/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */

import { URLExt } from '@jupyterlab/coreutils';
import { DefaultMindStudio } from './defaultMindStudio';

/**
 * The url for the mindstudio service.
 */
const MINDSTUDIO_SERVICE_URL = '';

const MINDSTUDIO_STATIC_CONFIG_URL = '';

const MINDSTUDIO_URL = '/resources/profiler/frontend/index.html';

const MINDSTUDIO_IFRAME_CONFIG_URL = '/mindstudio_insight_jupyterlab/get_iframe_config';

const MINDSTUDIO_TERMINATE_PROFILER_URL = '/mindstudio_insight_jupyterlab/terminate_profiler_server';

/**
 * A namespace for private data.
 */

/**
 * A mapping of running mindstudio by url.
 */
export const running: { [key: string]: DefaultMindStudio } =
    Object.create(null);

/**
 * Get the url for a mindstudio.
 */
export function getMindStudioUrl(baseUrl: string, name: string): string {
    return URLExt.join(baseUrl, MINDSTUDIO_SERVICE_URL, name);
}

/**
 * Get the url for a mindstudio.
 */
export function getMindStudioStaticConfigUrl(baseUrl: string): string {
    return URLExt.join(baseUrl, MINDSTUDIO_STATIC_CONFIG_URL);
}

/**
 * Get the base url.
 */
export function getServiceUrl(baseUrl: string): string {
    return URLExt.join(baseUrl, MINDSTUDIO_SERVICE_URL);
}

/**
 * Kill mindstudio by url.
*/
export function killMindStudio(url: string): void {
    // Update the local data store.
    if (running[url]) {
        const mindstudio = running[url];
        mindstudio.dispose();
    }
}

/**
 * Get the iframe config url.
 */
export function getIFrameConfigUrl(baseUrl: string): string {
    return URLExt.join(baseUrl, MINDSTUDIO_IFRAME_CONFIG_URL);
}

/**
 * Terminate profiler server.
 */
export function terminateIframe(baseUrl: string, profilerServerId: string): string {
    let url = URLExt.join(baseUrl, MINDSTUDIO_TERMINATE_PROFILER_URL);
    return `${url}?profilerServerId=${profilerServerId}`;
}

export function getMindStudioInstanceRootUrl(baseUrl: string): string {
    return URLExt.join(baseUrl, MINDSTUDIO_URL);
}

export function getMindStudioInstanceUrl(
    baseUrl: string,
    proxy: boolean,
    port: string,
    profilerServerId: string,
): string {
    let url = URLExt.join(baseUrl, MINDSTUDIO_URL);
    if (proxy) {
        return `${url}?jupyterlabProxy=true&port=${port}&profilerServerId=${profilerServerId}`;
    }
    return `${url}?port=${port}&profilerServerId=${profilerServerId}`;
}
