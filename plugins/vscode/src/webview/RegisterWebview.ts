import * as vscode from 'vscode';
import { Webview } from "./Webview";
import { spawn, exec, ChildProcess} from 'child_process';
import { join } from 'path';
import { readFileSync } from 'fs';
import { platform } from 'os';
import { WebviewManager } from './WebviewManager';

export class RegisterWebview extends Webview {

    private readonly _extensionUri: vscode.Uri;

    private htmlStr: string;
    private server?: ChildProcess;
    private serverCheckSchedule?: NodeJS.Timeout;
    private tryRestartTime = 0;
    private readonly port = 9001;
    private readonly findServerCommand = platform() === 'win32' ? 'tasklist | findstr profiler_server.exe' : 'ps aux | grep profiler_serve';

    constructor(viewType: string, title: string, context: vscode.ExtensionContext,  manager: WebviewManager) {
        super(viewType, title, context, manager);
        this._extensionUri = context.extensionUri;
        const filePath = join(__dirname, './profiler/index.html');
        this.htmlStr = readFileSync(filePath, 'utf8');
        this.startServer();
    }

    newPanel() {
        super.newPanel();
    }

    previewUIPage() {
        this.active();
        this.panel?.webview.postMessage(this._extensionUri.toString);
        (this.panel as vscode.WebviewPanel).webview.html = this.html();
    }

    startServer() {
        this.executeStartServerCommand();
        this.startServerCheckAndRestart();
    }

    executeStartServerCommand() {
        let serverName = './profiler/server/profiler_server';
        if (platform() === 'win32') {
            serverName = serverName + '.exe';
        }
        const serverPath = join(__dirname, serverName);
        if (platform() !== 'win32') {
            spawn('chmod', ['+x', serverPath]);
        }
        const serverCommand = serverPath;
        this.server = spawn(serverCommand, [' --wsPort=' + this.port]);
        this.server.stdout?.on('data', (data: any) => {
            console.log('[server][info]: ' + data);
        });
        this.server.stderr?.on('data', (data: any) => {
            console.log('[server][err]: ' + data);
        });
    }

    startServerCheckAndRestart() {
        this.serverCheckSchedule = setInterval(() => {
            exec(this.findServerCommand, (error, stdout, stderr) => {
                if (error) {
                    console.error(`exec error: ${error}`);
                }
                if (stderr) {
                    console.error(`exec stderr: ${stderr}`);
                }
                // server挂了,只提示3次
                if (!stdout.includes('profiler_server')) {
                    if (this.tryRestartTime++ < 3) {
                        vscode.window.showWarningMessage('[insight]: server has been dead, please close and reopen');
                        return;
                    } 
                    clearInterval(this.serverCheckSchedule);
                }
            });
        }, 3000);
    }

    dispose() {
        console.log('profiler is closed!');
        clearInterval(this.serverCheckSchedule);
        this.server?.kill();
        this.webviewManger.dispose();
    }

    html() {
        return this.htmlStr;
    }
}