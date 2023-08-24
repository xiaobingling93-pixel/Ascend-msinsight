import * as vscode from 'vscode';
import { Webview } from "./Webview";
import { spawn, exec } from 'child_process';
import { ChildProcess } from 'child_process';
import { join } from 'path';
import { platform } from 'os';
import { readFileSync } from 'fs';

export class RegisterWebview extends Webview {
    
    private readonly _extensionUri: vscode.Uri;
    
    private readonly nameList: any;
    
    private server?: ChildProcess;
    private serverCheckSchedule?: NodeJS.Timeout;
    private hasBeenDead = false;
    private tryRestartTime = 0;
    private readonly findServerCommand = platform() === 'win32' ? 'tasklist | findstr profiler-server.exe' : 'ps aux | grep profiler-serve';

    constructor(viewType: string, title: string, context: vscode.ExtensionContext) {
        super(viewType, title, context);
        this._extensionUri = context.extensionUri;
        let data = readFileSync(join(__dirname, './profiler/asset-manifest.json'));
        let result = data.toString();
        const jsonObject = JSON.parse(result);
        this.nameList = jsonObject;
        this.startServer();
    }

    newPanel() {
        super.newPanel();
    }

    entry() {
        const entryPath = vscode.Uri.joinPath(this._extensionUri, 'profiler', this.nameList.files['main.js']);
        return this.panel?.webview.asWebviewUri(entryPath);
    }

    style() {
        const entryPath = vscode.Uri.joinPath(this._extensionUri, 'profiler', this.nameList.files['main.css']);
        return this.panel?.webview.asWebviewUri(entryPath);
    }
    manifest() {
        const manifestPath = vscode.Uri.joinPath(this._extensionUri, 'profiler', 'manifest.json');
        return this.panel?.webview.asWebviewUri(manifestPath); 
    }
    icon() {
        const icontPath = vscode.Uri.joinPath(this._extensionUri, 'profiler', 'logo192.png');
        return this.panel?.webview.asWebviewUri(icontPath);
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
        const entryName = './profiler/server/resources/app/app.js';
        const entryWholeName = join(__dirname, entryName);
        let serverName = './profiler/server/profiler-server';
        if (platform() === 'win32') {
            serverName = serverName + '.exe';
        }
        const serverPath = join(__dirname, serverName);
        if (platform() !== 'win32') {
            spawn('chmod', ['+x', serverPath]);
        }
        this.server = spawn(serverPath, [entryWholeName]);
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
                // server挂了,只提醒3次
                if (!stdout.includes('profiler-server')) {
                    if (this.tryRestartTime++ < 3) {
                        vscode.window.showWarningMessage('[insight]: server has been dead, please close and reopen');
                        return;
                    } 
                }
            });
        }, 3000);
    }

    dispose() {
        console.log('profiler is closed!!');
        clearInterval(this.serverCheckSchedule);
        this.server?.kill();
    }

    // rem单位相对于font-size取值，设为6px，设计稿为750px时，设置初始UI宽度缩放100%时450px，6*750/450=10px=1rem，默认缩放比例为75%，初始时6*0.75=4.5
    html() {
        return `<!doctype html>
                <html lang="en">

                <head>
                    <meta charset="utf-8" />
                    <link rel="icon" href="/favicon.png" />
                    <meta name="viewport" content="width=device-width,initial-scale=1" />
                    <meta name="theme-color" content="#000000" />
                    <meta name="description" content="Web insight demo project" />
                    <link rel="apple-touch-icon" href="${this.icon()}" />
                    <link rel="manifest" href="${this.manifest}" />
                    <title>Web Insight</title>
                    <script defer="defer" src="${this.entry()}"></script>
                    <link href="${this.style()}" rel="stylesheet">
                    <style>
                        #body {
                            padding: 0;
                        }
                    </style>
                </head>
                <body id="body"><noscript>You need to enable JavaScript to run this app.</noscript>
                <script>
                console.log('this.entry()');
                console.log(${this.entry()});
                </script>
                    <div id="root"></div>
                </body>
                </html>
        `;
    }
}