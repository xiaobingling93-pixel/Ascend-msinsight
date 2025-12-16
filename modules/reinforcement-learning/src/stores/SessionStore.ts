/*
 * -------------------------------------------------------------------------
 * This file is part of the MindStudio project.
 * Copyright (c) 2025 Huawei Technologies Co.,Ltd.
 *
 * MindStudio is licensed under Mulan PSL v2.
 * You can use this software according to the terms and conditions of the Mulan PSL v2.
 * You may obtain a copy of Mulan PSL v2 at:
 *
 *          http://license.coscl.org.cn/MulanPSL2
 *
 * THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
 * EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
 * MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
 * See the Mulan PSL v2 for more details.
 * -------------------------------------------------------------------------
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
