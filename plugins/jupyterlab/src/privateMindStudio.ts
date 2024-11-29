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

const MINDSTUDIO_URL = '';

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

export function getMindStudioInstanceRootUrl(baseUrl: string): string {
    return URLExt.join(baseUrl, MINDSTUDIO_URL);
}

export function getMindStudioInstanceUrl(
    baseUrl: string,
    name: string
): string {
    return URLExt.join(baseUrl, MINDSTUDIO_URL, name);
}
