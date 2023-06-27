'use strict';

import * as vscode from 'vscode';
import { RegisterWebview } from './webview/RegisterWebview';
import { Webview } from './webview/Webview';

export function activate(context: vscode.ExtensionContext) {
	context.subscriptions.push(
		vscode.commands.registerCommand('ascend-insight.start', () => {
			let webview = new RegisterWebview('insight', 'insight', context);
			webview.previewUIPage();
		})
	);
}