/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */

import { JupyterFrontEnd } from '@jupyterlab/application';
import { ReactWidget } from '@jupyterlab/apputils';
import React from 'react';
import * as MindStudio from '../mindstudio';
import { Message } from '@lumino/messaging';
import { MindStudioManager } from '../manager';
import * as CommandIDs from '../commands';

export interface MindStudioInvokeOptions {
  mindstudioManager: MindStudioManager;
  createdModelName?: string;
  app: JupyterFrontEnd;
}

/**
 * A Counter Lumino Widget that wraps a CounterComponent.
 */
export class MindStudioReactWidget extends ReactWidget {
  mindstudioManager: MindStudioManager;
  app: JupyterFrontEnd;

  currentMindStudioModel: MindStudio.IModel | null = null;
  createdModelName?: string;

  /**
   * Constructs a new CounterWidget.
   */
  constructor(options: MindStudioInvokeOptions) {
    super();
    this.mindstudioManager = options.mindstudioManager;
    this.createdModelName = options.createdModelName;
    this.app = options.app;

    this.title.closable = true;
    this.title.label = 'MindStudio Insight';
    this.title.caption = `Name: ${this.title.label}`;
  }

  dispose(): void {
    super.dispose();
  }

  closeCurrent = (): void => {
    this.dispose();
    this.close();
  };

  startNew = (
    name: string,
    options?: MindStudio.IOptions
  ): Promise<MindStudio.IMindStudio> => {
    return this.mindstudioManager.startNew(
      name,
      options
    );
  };

  setWidgetName = (name: string): void => {
    this.title.label = name || 'MindStudio Insight';
    this.title.caption = `Name: ${this.title.label}`;
  };

  render(): JSX.Element {
    return (
      <></>
    );
  };

  protected updateCurrentModel = (model: MindStudio.IModel | null): void => {
    this.currentMindStudioModel = model;
  };

  protected onCloseRequest(msg: Message): void {
    super.onCloseRequest(msg);
    this.dispose();
  }

  protected openMindStudio = (modelName: string): void => {
    this.app.commands.execute(CommandIDs.open, {
      modelName,
    });
  };

  protected openDoc = (): void => {
    this.app.commands.execute(CommandIDs.openDoc);
  };
}
