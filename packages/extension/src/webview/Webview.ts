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

        const openChooseDialog = async (canSelectFolders: boolean, canSelectFiles: boolean) => {
            const options: vscode.OpenDialogOptions = {
                canSelectMany: false,
                canSelectFolders: canSelectFolders,
                canSelectFiles: canSelectFiles,
                openLabel: '确定选择',
                filters: {
                    'JSON files': ['json']
                }
            };
            const result = await vscode.window.showOpenDialog(options);
            if (result && result.length > 0) {
                const folderPath = result[0].fsPath;
                this.panel?.webview.postMessage({command: 'ascend.folderSelected', path: folderPath});
            } else {
                this.panel?.webview.postMessage({command: 'ascend.folderSelectionCanceled'});
            }
        };

        this.panel.webview.onDidReceiveMessage(
            async message => {
                switch (message.command) {
                    case 'ascend.selectFolder':
                        await openChooseDialog(true, false);
                        break;
                    case 'ascend.selectFile':
                        await openChooseDialog(false, true);
                        break;
                }
            },
            undefined,
            this.context.subscriptions
        );

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