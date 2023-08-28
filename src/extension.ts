'use strict';

import * as vscode from 'vscode';
import { RegisterWebview } from './webview/RegisterWebview';
export let singleWebview: RegisterWebview;
export function activate(context: vscode.ExtensionContext) {
	context.subscriptions.push(
		vscode.commands.registerCommand('ascend-insight.start', () => {
			if (singleWebview === undefined) {
				singleWebview =  new RegisterWebview('insight', 'insight', context);
			}
			singleWebview.previewUIPage();
		})
	);
}