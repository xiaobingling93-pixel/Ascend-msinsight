/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
*/
import * as vscode from 'vscode';
import { Webview } from './Webview';
import { spawn, type ChildProcess, spawnSync} from 'child_process';
import { join } from 'path';
import { readFileSync } from 'fs';
import { platform } from 'os';
import type { WebviewManager } from './WebviewManager';

const modules = ['./profiler/plugins/Timeline/index.html',
    './profiler/plugins/Memory/index.html',
    './profiler/plugins/Cluster/summary.html',
    './profiler/plugins/Cluster/communication.html',
];

export class RegisterWebview extends Webview {
    private readonly _extensionUri: vscode.Uri;
    private htmlStr: string;
    private server?: ChildProcess;
    private serverCheckSchedule?: NodeJS.Timeout;
    private tryRestartTime = 0;
    private port = 9000;

    constructor(viewType: string, title: string, context: vscode.ExtensionContext, manager: WebviewManager) {
        super(viewType, title, context, manager);
        this._extensionUri = context.extensionUri;
        const filePath = join(__dirname, './profiler/index.html');
        this.htmlStr = readFileSync(filePath, 'utf8');
        this.startServer();
    }

    newPanel(): void {
        super.newPanel();
    }

    previewUIPage(): void {
        this.active();
        this.panel?.webview.postMessage(this._extensionUri.toString);
        (this.panel as vscode.WebviewPanel).webview.html = this.html();
        this.panel?.webview.postMessage({ event: 'updateHtml',
            modules: modules.map(path => this.getModulesHtml(path)), port: this.port});
    }

    startServer(): void {
        this.executeStartServerCommand();
        this.startServerCheckAndRestart();
    }

    executeStartServerCommand(): void {
        let serverName = './profiler/server/profiler_server';
        if (platform() === 'win32') {
            serverName = `${serverName}.exe`;
        }
        const serverPath = join(__dirname, serverName);
        if (platform() !== 'win32') {
            spawn('chmod', ['+x', serverPath]);
        }
        const serverCommand = serverPath;

        const port = this.scanPort(serverCommand);

        if (!port) {
            vscode.window.showWarningMessage('[insight]: Can\'t find available port');
            return;
        }
        this.port = Number.parseInt(port);
        this.server = spawn(serverCommand, [` --wsPort=${this.port}`]);
    }

    scanPort(serverCommand: string): undefined | string {
        const scanPort = spawnSync(serverCommand, [`--scan=${this.port}`]);
        if (scanPort.error) {
            return undefined;
        }
        const output = scanPort.output.toString();
        if (output.indexOf('Available port: ') !== -1) {
            const result = output.match(/d+/);
            if (result) {
                return result.pop();
            }
            return undefined;
        }
        return undefined;
    }

    startServerCheckAndRestart(): void {
        this.serverCheckSchedule = setInterval(() => {
            if (this.server?.exitCode !== null && this.panel?.active) {
                if (this.tryRestartTime++ < 3) {
                    vscode.window.showWarningMessage('[insight]: server has been dead, please close and reopen');
                    return;
                }
                clearInterval(this.serverCheckSchedule);
            }
        }, 3000);
    }

    dispose(): void {
        clearInterval(this.serverCheckSchedule);
        this.server?.kill();
        this.webviewManger.dispose();
    }

    html(): string {
        return this.htmlStr;
    }
    
    getModulesHtml(path: string): string {
        return readFileSync(join(__dirname, path), 'utf8');
    }
}