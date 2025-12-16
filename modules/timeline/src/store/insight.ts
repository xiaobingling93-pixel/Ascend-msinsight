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
import { action, makeAutoObservable } from 'mobx';
import type { InsightTemplate } from '../entity/insight';
import { entryTemplate } from '../insight/templates/entry';

export class InsightStore {
    readonly templates: Map<string, InsightTemplate>;

    constructor() {
        makeAutoObservable(this);
        this.templates = new Map<string, InsightTemplate>();
    }

    get(id: string): InsightTemplate | undefined {
        return this.templates.get(id);
    }

    has(id: string): boolean {
        return this.templates.has(id);
    }

    loadTemplates(): Promise<void> {
        this.templates.clear();
        const list = [entryTemplate];
        return new Promise<void>(resolve => {
            setTimeout(action(() => {
                list.forEach(template => {
                    this.templates.set(template.id, template);
                });
                resolve();
            }), 0);
        });
    }
}
