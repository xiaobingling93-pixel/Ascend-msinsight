/*
 * -------------------------------------------------------------------------
 * This file is part of the MindStudio project.
 * Copyright (c) 2025 Huawei Technologies Co.,Ltd.
 *
 * MindStudio is licensed under Mulan PSL v2.
 * You can use this software according to the terms and conditions of the Mulan PSL v2.
 * You may obtain a copy of Mulan PSL v2 at:
 *
 *          http://license.coscl.org.cn/MulanPSL2
 *
 * THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
 * EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
 * MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
 * See the Mulan PSL v2 for more details.
 * -------------------------------------------------------------------------
 */

import * as staticMindStudio from './staticMindStudio';
import { IDisposable } from './utils/lumino/disposable';
import { ISignal } from './utils/lumino/signaling';
import { JSONObject } from './utils/lumino/coreutils';
import { ServerConnection } from '@jupyterlab/services';

/**
 * The namespace for mindstudio statics.
 */

/**
 * An interface for a mindstudio.
 */
export interface IMindStudio extends IDisposable {
  /**
   * A signal emitted when the mindstudio is shut down.
   */
  terminated: ISignal<IMindStudio, void>;

  /**
   * The model associated with the mindstudio.
   */
  readonly model: IModel;

  /**
   * Get the name of the mindstudio.
   */
  readonly name: string;

  /**
   * The server settings for the mindstudio.
   */
  readonly serverSettings: ServerConnection.ISettings;

  /**
   * Shut down the mindstudio.
   */
  shutdown: () => Promise<void>;
}

/**
 * The options for intializing a mindstudio object.
 */
export interface IOptions {
  /**
   * The server settings for the mindstudio.
   */
  serverSettings?: ServerConnection.ISettings;
}

/**
 * The server model for a mindstudio.
 */
export interface IModel extends JSONObject {
  /**
   * The name of the mindstudio.
   */
  readonly name: string;
}

export interface IStaticConfig extends JSONObject {
  /**
   * The name of the mindstudio.
   */
  readonly notebookDir: string;
}

export function getStaticConfig(
  settings?: ServerConnection.ISettings
): Promise<IStaticConfig> {
  return staticMindStudio.getStaticConfig(settings);
}

export function startNew(
  name: string,
  options?: IOptions
): Promise<IMindStudio> {
  return staticMindStudio.startNew(name, options);
}

export function listRunning(
  settings?: ServerConnection.ISettings
): Promise<IModel[]> {
  return staticMindStudio.listRunning(settings);
}

export function shutdown(
  name: string,
  settings?: ServerConnection.ISettings
): Promise<void> {
  return staticMindStudio.shutdown(name, settings);
}

export function shutdownAll(
  settings?: ServerConnection.ISettings
): Promise<void> {
  return staticMindStudio.shutdownAll(settings);
}

export async function startIframeUrl(
  settings?: ServerConnection.ISettings
): Promise<string> {
  return staticMindStudio.startIframeUrl(settings);
}

export async function terminateIframe(
  profilerServerId: string,
  settings?: ServerConnection.ISettings
): Promise<void> {
  return staticMindStudio.terminateIframe(profilerServerId, settings);
}

/**
 * The interface for a mindstudio manager.
 *
 * The manager is respoonsible for maintaining the state of running
 * mindstudio.
 */
export interface IManager extends IDisposable {
  readonly serverSettings: ServerConnection.ISettings;

  runningChanged: ISignal<this, IModel[]>;

  running: () => Array<IModel>;

  startNew: (name: string, options?: IOptions) => Promise<IMindStudio>;

  shutdown: (name: string) => Promise<void>;

  shutdownAll: () => Promise<void>;

  refreshRunning: () => Promise<void>;
}
