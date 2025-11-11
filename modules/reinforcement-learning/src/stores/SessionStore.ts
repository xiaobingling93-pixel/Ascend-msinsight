/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2025-2025. All rights reserved.
 */

import { makeAutoObservable } from 'mobx';
import { RootStore } from './RootStore';
import i18n from '@insight/lib/i18n';

export type LocaleType = 'zhCN' | 'enUS';

export class SessionStore {
    rootStore: RootStore;
    currentLocale: LocaleType = 'enUS';
    unitcount: number = 0;
    parseCompleted: boolean = false;

    constructor(rootStore: RootStore) {
        this.rootStore = rootStore;
        makeAutoObservable(this);
    }

    updateField<K extends keyof this>(key: K, value: this[K]): void {
        if (Object.prototype.hasOwnProperty.call(this, key)) {
            this[key] = value;
        }
    }

    setLocale(locale: LocaleType): void {
        this.currentLocale = locale;
        i18n.changeLanguage(locale);
    }

    reset(): void {
        this.unitcount = 0;
        this.parseCompleted = false;
    }
}
