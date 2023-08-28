import * as vscode from 'vscode';
import { Webview } from "./Webview";
import { join } from 'path';
import { readFileSync } from 'fs';

export class RegisterWebview extends Webview {
    
    private readonly _extensionUri: vscode.Uri;
    
    private readonly nameList: any;

    constructor(viewType: string, title: string, context: vscode.ExtensionContext) {
        super(viewType, title, context);
        this._extensionUri = context.extensionUri;
        let data = readFileSync(join(__dirname, './profiler/asset-manifest.json'));
        let result = data.toString();
        const jsonObject = JSON.parse(result);
        this.nameList = jsonObject;
    }

    newPanel() {
        super.newPanel();
    }

    entry() {
        // const entryPath = vscode.Uri.joinPath(this._extensionUri, 'vue-dist', 'index.js');
        const entryPath = vscode.Uri.joinPath(this._extensionUri, 'profiler', this.nameList.files['main.js']);
        return this.panel?.webview.asWebviewUri(entryPath);
    }

    style() {
        // const entryPath = vscode.Uri.joinPath(this._extensionUri, 'vue-dist', 'index.css');
        const entryPath = vscode.Uri.joinPath(this._extensionUri, 'profiler', this.nameList.files['main.css']);
        return this.panel?.webview.asWebviewUri(entryPath);
    }

    previewUIPage() {
        this.active();
        this.panel?.webview.postMessage(this._extensionUri.toString);
        (this.panel as vscode.WebviewPanel).webview.html = this.html();
    }

    dispose() {
        console.log('profiler is closed!');
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
                    <meta name="description" content="ascend insight webview" />
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
                <div id="root"></div>
                </body>
                </html>
        `;
    }
}