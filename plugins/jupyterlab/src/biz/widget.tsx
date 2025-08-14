/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */

import { JupyterFrontEnd } from '@jupyterlab/application';
import { ReactWidget } from '@jupyterlab/apputils';
import React from 'react';
import * as MindStudio from '../mindstudio';
import { MindStudioManager } from '../manager';
import * as CommandIDs from '../commands';
import { MindStudioInsightTab } from './MindStudioInsight';

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
  profilerServerId: string | null;

  /**
   * Constructs a new CounterWidget.
   */
  constructor(options: MindStudioInvokeOptions) {
    super();
    this.mindstudioManager = options.mindstudioManager;
    this.createdModelName = options.createdModelName;
    this.app = options.app;
    this.profilerServerId = '';

    this.title.closable = true;
    this.title.label = 'MindStudio Insight';
    this.title.caption = `Name: ${this.title.label}`;
  }

  dispose(): void {
    this.mindstudioManager.terminateIframe(this.profilerServerId ?? '').then(() => {})
      .catch(() => {});
    super.dispose();
  }

  closeCurrent = (): void => {
    this.dispose();
    this.close();
  };

  startIframeUrl = async (): Promise<string> => {
    const url = await this.mindstudioManager.startIframeUrl();
    const match = url.match(/profilerServerId=(?<profilerServerId>[^&;]+)/);
    this.profilerServerId = match ? match[1] : null;
    return url;
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
      <MindStudioInsightTab
        mindstudioManager={this.mindstudioManager}
        closeWidget={this.closeCurrent}
        openMindStudio={this.openMindStudio}
        updateCurrentModel={this.updateCurrentModel}
        startNew={this.startNew}
        startIFrame={this.startIframeUrl}
      />
    );
  };

  protected updateCurrentModel = (model: MindStudio.IModel | null): void => {
    this.currentMindStudioModel = model;
  };

  protected openMindStudio = (modelName: string): void => {
    this.app.commands.execute(CommandIDs.open, {
      modelName,
    });
  };
}
