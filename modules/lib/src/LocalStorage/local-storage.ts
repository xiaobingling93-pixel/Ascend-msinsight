/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */
import { customConsole as console, safeJSONParse } from 'ascend-utils';
export enum LocalStorageKey {
    LANGUAGE = 'language',
    THEME = 'theme',
    LAST_FILE_PATH = 'last_file_path',
}

class LocalStorageService {
    getItem(key: LocalStorageKey): any {
        const item = localStorage.getItem(key);
        return item !== null ? safeJSONParse(item) : null;
    }

    setItem(key: LocalStorageKey, value: any): void {
        try {
            localStorage.setItem(key, JSON.stringify(value));
        } catch (error) {
            console.log(error);
        }
    }

    removeItem(key: LocalStorageKey): void {
        localStorage.removeItem(key);
    }

    clear(): void {
        localStorage.clear();
    }
}

export const localStorageService = new LocalStorageService();
export default localStorageService;
