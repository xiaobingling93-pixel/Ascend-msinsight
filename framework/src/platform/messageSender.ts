import { Browser } from './Browser';
import { IntellijPlatform } from './IntellijPlatform';
import { VsCodePlatform } from './VsCodePlatform';
import { BasePlatform } from './BasePlatform';

declare function acquireVsCodeApi(): any;

export let MESSAGE_SENDER: BasePlatform = new Browser();
if (typeof acquireVsCodeApi === 'function') {
    MESSAGE_SENDER = new VsCodePlatform();
    MESSAGE_SENDER.sendMessage = acquireVsCodeApi().postMessage;
}

if (typeof window.cefQuery === 'function') {
    MESSAGE_SENDER = new IntellijPlatform();
}
