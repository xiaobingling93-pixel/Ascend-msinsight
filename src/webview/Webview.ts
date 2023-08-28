import path = require('path');
import * as vscode from 'vscode';
export abstract class Webview {
    viewType: string;

    title: string;

    panel!: vscode.WebviewPanel | undefined;
    context: vscode.ExtensionContext;

    constructor(viewType: string, title: string, context: vscode.ExtensionContext) {
        this.viewType = viewType;
        this.title = title;
        this.context = context;
    }

    newPanel() {
        this.panel = vscode.window.createWebviewPanel(this.viewType, this.title, {
            viewColumn: vscode.ViewColumn.One,
            preserveFocus: true,
        }, {
            enableScripts: true,
            retainContextWhenHidden: true,
            localResourceRoots: [vscode.Uri.file(path.join(this.context.extensionPath, 'profiler'))]
        });
        this.panel.onDidDispose(() => {
            if (this.panel !== undefined) {
                this.panel.dispose();
                this.panel = undefined;
            }
            this.dispose();
        });
    }

    active() {
        if (this.panel !== undefined) {
            this.panel.reveal(vscode.ViewColumn.One, true);
            return;
        }
        this.newPanel();
    }

    dispose() {
        console.log('profiler is closed');
    }
}