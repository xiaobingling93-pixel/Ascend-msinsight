/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */
import { console } from '@/utils/console';
import { safeJSONParse } from '.';
export enum LocalStorageKeys {
    LANGUAGE = 'language',
    THEME = 'theme',
    LAST_FILE_PATH = 'last_file_path',
}

class LocalStorageService {
    getItem(key: LocalStorageKeys): any {
        const item = localStorage.getItem(key);
        return item ? safeJSONParse(item) : null;
    }

    setItem(key: LocalStorageKeys, value: any): void {
        try {
            localStorage.setItem(key, JSON.stringify(value));
        } catch (error) {
            console.log(error);
        }
    }

    removeItem(key: LocalStorageKeys): void {
        localStorage.removeItem(key);
    }

    clear(): void {
        localStorage.clear();
    }
}

export const localStorageService = new LocalStorageService();
export default localStorageService;
