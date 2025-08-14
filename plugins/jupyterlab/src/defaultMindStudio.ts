/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */

import * as MindStudio from './mindstudio';
import * as Private from './privateMindStudio';
import * as staticMindStudio from './staticMindStudio';
import { Signal } from './utils/lumino/signaling';
import { ServerConnection } from '@jupyterlab/services';

export class DefaultMindStudio implements MindStudio.IMindStudio {
    readonly serverSettings: ServerConnection.ISettings;
    private _name: string;
    private _terminated = new Signal<this, void>(this);
    private _isDisposed = false;
    private _runningUrl: string;

    /**
     * constructor a new mindstudio
     */
    constructor(name: string, options: MindStudio.IOptions = {}) {
        this._name = name;
        this.serverSettings =
            options.serverSettings || ServerConnection.makeSettings();
        this._runningUrl = Private.getMindStudioInstanceUrl(this.serverSettings.baseUrl, false, '9000', '');
    }

    get name(): string {
        return this._name;
    }

    get model(): MindStudio.IModel {
        return {
            name: this._name,
        };
    }

    get terminated(): Signal<this, void> {
        return this._terminated;
    }

    get isDisposed(): boolean {
        return this._isDisposed;
    }

    dispose(): void {
        if (this._isDisposed) {
            return;
        }

        this.terminated.emit(void 0);
        this._isDisposed = true;
        delete Private.running[this._runningUrl];
        Signal.clearData(this);
    }

    shutdown(): Promise<void> {
        const { name, serverSettings } = this;
        return staticMindStudio.shutdown(name, serverSettings);
    }
}