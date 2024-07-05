import { RegisterWebview } from "./RegisterWebview";
import { Webview } from "./Webview";
import * as vscode  from 'vscode';


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