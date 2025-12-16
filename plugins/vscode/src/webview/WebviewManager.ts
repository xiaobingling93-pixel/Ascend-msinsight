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

import { RegisterWebview } from './RegisterWebview';
import type { Webview } from './Webview';
import type * as vscode from 'vscode';


export class WebviewManager {
    private webview?: Webview;

    createWebview(viewType: string, title: string, context: vscode.ExtensionContext): Webview {
        if (this.webview === undefined) {
            this.webview = new RegisterWebview(viewType, title, context, this);
        }
        return this.webview;
    }
    
    dispose(): void {
        this.webview = undefined;
    }
}

export const webviewManager = new WebviewManager();