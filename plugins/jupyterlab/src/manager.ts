/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */

import { ArrayExt } from '@lumino/algorithm';
import { Signal, ISignal } from '@lumino/signaling';
import { JSONExt } from '@lumino/coreutils';
import * as MindStudio from './mindstudio';
import * as staticManager from './staticManager';
import { ServerConnection } from '@jupyterlab/services';

/**
 * A mindstudio manager.
 */
export class MindStudioManager implements MindStudio.IManager {
  readonly serverSettings: ServerConnection.ISettings;
  getStaticConfigPromise: Promise<void>;
  private _models: MindStudio.IModel[] = [];
  private _mindstudios = new Set<MindStudio.IMindStudio>();
  private _isDisposed = false;
  private _isReady = false;
  private _readyPromise: Promise<void>;
  private _runningChanged = new Signal<this, MindStudio.IModel[]>(this);
  private _statusConfig: MindStudio.IStaticConfig | null = null;

  /**
   * Construct a new mindstudio manager.
   */
  constructor(options: staticManager.IOptions = {}) {
    this.serverSettings =
      options.serverSettings || ServerConnection.makeSettings();
    this._readyPromise = this._refreshRunning();
    this.getStaticConfigPromise = this._getStaticConfig();
  }

  get runningChanged(): ISignal<this, MindStudio.IModel[]> {
    return this._runningChanged;
  }

  get isDisposed(): boolean {
    return this._isDisposed;
  }

  get isReady(): boolean {
    return this._isReady;
  }

  get ready(): Promise<void> {
    return this._readyPromise;
  }

  dispose(): void {
    if (this.isDisposed) {
      return;
    }
    this._isDisposed = true;
    Signal.clearData(this);
    this._models = [];
  }

  /**
   * Create an iterator over the most recent running mindstudio.
   *
   * @returns A new iterator over the running mindstudio.
   */
  running(): Array<MindStudio.IModel> {
    return this._models;
  }

  /**
   * Create a new mindstudio.
   */
  async startNew(
    name: string,
    options?: MindStudio.IOptions
  ): Promise<MindStudio.IMindStudio> {
    const mindStudio = await MindStudio.startNew(
      name,
      this._getOptions(options)
    );
    this._onStarted(mindStudio);
    return mindStudio;
  }

  /**
   * Shut down a mindstudio by name.
   */
  async shutdown(name: string): Promise<void> {
    const index = ArrayExt.findFirstIndex(
      this._models,
      (value: MindStudio.IModel) => value.name === name
    );
    if (index === -1) {
      return;
    }

    this._models.splice(index, 1);
    this._runningChanged.emit(this._models.slice());

    try {
      await MindStudio.shutdown(name, this.serverSettings);
      const toRemove: MindStudio.IMindStudio[] = [];
      this._mindstudios.forEach((t: MindStudio.IMindStudio) => {
        if (t.name === name) {
          t.dispose();
          toRemove.push(t);
        }
      });
      toRemove.forEach(s => {
        this._mindstudios.delete(s);
      });
    } catch (error) {
      throw error;
    }
  }

  /**
   * Shut down all mindstudio.
   *
   * @returns A promise that resolves when all of the mindstudio are shut down.
   */
  async shutdownAll(): Promise<void> {
    const models = this._models;
    if (models.length > 0) {
      this._models = [];
      this._runningChanged.emit([]);
    }
    try {
      await this._refreshRunning();
      const toRemove: MindStudio.IMindStudio[] = [];
      for (const model of models) {
        await MindStudio.shutdown(model.name, this.serverSettings);
        this._mindstudios.forEach((t: MindStudio.IMindStudio) => {
          t.dispose();
          toRemove.push(t);
        });
        toRemove.forEach(t => {
          this._mindstudios.delete(t);
        });
      }
      return undefined;
    } catch (error) {
      throw error;
    }
  }

  refreshRunning(): Promise<void> {
    return this._refreshRunning();
  }

  private _onTerminated(name: string): void {
    const index = ArrayExt.findFirstIndex(
      this._models,
      (value: MindStudio.IModel) => value.name === name
    );
    if (index !== -1) {
      this._models.splice(index, 1);
      this._runningChanged.emit(this._models.slice());
    }
  }

  private _onStarted(mindstudio: MindStudio.IMindStudio): void {
    const name = mindstudio.name;
    this._mindstudios.add(mindstudio);
    const index = ArrayExt.findFirstIndex(
      this._models,
      (value: MindStudio.IModel) => value.name === name
    );
    if (index === -1) {
      this._models.push(mindstudio.model);
      this._runningChanged.emit(this._models.slice());
    }
    mindstudio.terminated.connect(() => {
      this._onTerminated(name);
    });
  }

  private async _refreshRunning(): Promise<void> {
    const models = await MindStudio.listRunning(this.serverSettings);
    this._isReady = true;
    if (!JSONExt.deepEqual(models, this._models)) {
      const names = models.map(r => r.name);
      const toRemove: MindStudio.IMindStudio[] = [];
      for (const t of this._mindstudios) {
        if (!names.includes(t.name)) {
          t.dispose();
          toRemove.push(t);
        }
      }
      for (const t of toRemove) {
        this._mindstudios.delete(t);
      }
      this._models = models.slice();
      this._runningChanged.emit(models);
    }
  }

  private _getOptions(options: MindStudio.IOptions = {}): MindStudio.IOptions {
    return { ...options, serverSettings: this.serverSettings };
  }

  private _getStaticConfig(): Promise<void> {
    return MindStudio.getStaticConfig(this.serverSettings).then(config => {
      if (this._statusConfig?.notebookDir) {
        this._statusConfig = config;
      }
    });
  }
}
