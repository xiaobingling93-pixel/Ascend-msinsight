/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */
import { Console as console } from '@/utils/console';
export enum LocalStorageKeys {
    LANGUAGE = 'language',
    THEME = 'theme',
}

class LocalStorageService {
    getItem(key: LocalStorageKeys): any {
        try {
            const item = localStorage.getItem(key);
            return item ? JSON.parse(item) : null;
        } catch (error) {
            console.log(error);
            return null;
        }
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
