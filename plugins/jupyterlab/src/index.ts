/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */

import {
  JupyterFrontEnd,
  JupyterFrontEndPlugin
} from '@jupyterlab/application';
import {
  ICommandPalette,
  showDialog,
  Dialog,
  IWidgetTracker,
  MainAreaWidget,
  WidgetTracker
} from '@jupyterlab/apputils';
import { ILauncher } from '@jupyterlab/launcher';
import { IMainMenu } from '@jupyterlab/mainmenu';
import { IRunningSessionManagers, IRunningSessions } from '@jupyterlab/running';
import { LabIcon } from '@jupyterlab/ui-components';
import mindstudioSvgStr from '../style/mindstudio.svg';
import * as MindStudio from './mindstudio';
import { MindStudioManager } from './manager';
import * as CommandIDs from './commands';
import { MindStudioReactWidget } from './biz/widget';

const mindstudioIcon = new LabIcon({
  name: 'jupyterlab-mindstudio-insight:mindstudio',
  svgstr: mindstudioSvgStr,
});

/**
 * Initialization data for the jupyterlab_mindstudio_insight extension.
 */
const plugin: JupyterFrontEndPlugin<IWidgetTracker<MainAreaWidget<MindStudioReactWidget>>> = {
  id: 'jupyterlab_mindstudio_insight:plugin',
  requires: [ICommandPalette as any],
  optional: [ILauncher as any, IMainMenu, IRunningSessionManagers],
  autoStart: true,
  activate: activate,
};

export default plugin;

async function activate(
  app: JupyterFrontEnd,
  palette: ICommandPalette,
  launcher: ILauncher | null,
  menu: IMainMenu | null,
  runningSessionManagers: IRunningSessionManagers | null
): Promise<WidgetTracker<MainAreaWidget<MindStudioReactWidget>>> {
  const manager = new MindStudioManager();
  const namespace = 'mindstudio';
  const tracker = new WidgetTracker<MainAreaWidget<MindStudioReactWidget>>({
    namespace,
  });

  addCommands(app, manager, tracker, launcher, menu);
  if (runningSessionManagers) {
    await addRunningSessionManager(runningSessionManagers, app, manager);
  }

  palette.addItem({ command: CommandIDs.openDoc, category: 'MindStudio' });

  return tracker;
}

function addCommands(
  app: JupyterFrontEnd,
  manager: MindStudioManager,
  tracker: WidgetTracker<MainAreaWidget<MindStudioReactWidget>>,
  launcher: ILauncher | null,
  menu: IMainMenu | null
): void {
  const { commands } = app;
  addOpenCommand(commands, app, manager, tracker);
  addOpenDocCommand(commands);
  addCloseCommand(commands, tracker);
  addCreateNewCommand(commands, app);

  if (launcher) {
    launcher.add({
      rank: 2,
      command: CommandIDs.createNew,
      category: 'Other',
    });
  }

  if (menu) {
    menu.fileMenu.newMenu.addGroup(
      [
        {
          command: CommandIDs.createNew,
        },
      ],
      30
    );
  }
}

function addOpenCommand(
  commands: any,
  app: JupyterFrontEnd,
  manager: MindStudioManager,
  tracker: WidgetTracker<MainAreaWidget<MindStudioReactWidget>>
): void {
  commands.addCommand(CommandIDs.open, {
    execute: (args: any) => {
      let modelName = args.modelName as string | undefined;
      let widget: MainAreaWidget<MindStudioReactWidget> | null | undefined = null;

      // step1: find an opened widget
      if (!modelName) {
        widget = tracker.find((t: MainAreaWidget<MindStudioReactWidget>) => {
          return (
            t.content.createdModelName === undefined
          );
        });
      }
      return getWidget(modelName, widget, app, manager, tracker);
    },
  });
}

function getWidget(
  modelName: any,
  widget: any,
  app: JupyterFrontEnd,
  manager: MindStudioManager,
  tracker: WidgetTracker<MainAreaWidget<MindStudioReactWidget>>
): MainAreaWidget<MindStudioReactWidget> {
  if (widget) {
    app.shell.activateById(widget.id);
    return widget;
  } else {
    let newModelName = '';
    if (!modelName) {
      const runningMindStudios = [...manager.running()];
      for (const model of runningMindStudios) {
        newModelName = model.name;
      }
    }
    const tabReact = new MindStudioReactWidget({
      mindstudioManager: manager,
      app,
      createdModelName: newModelName,
    });
    const tabWidget = new MainAreaWidget({ content: tabReact });
    tracker.add(tabWidget);
    app.shell.add(tabWidget, 'main');
    app.shell.activateById(tabWidget.id);
    return tabWidget;
  }
}

function addOpenDocCommand(commands: any): void {
  commands.addCommand(CommandIDs.openDoc, {
    execute: (args: any) => {
      window.open('https://www.hiascend.com/document/detail/zh/mindstudio/70RC3/msinsightug/msascendinsightug/Insight_userguide_0002.html');
    },
  });
}

function addCloseCommand(commands: any, tracker: WidgetTracker<MainAreaWidget<MindStudioReactWidget>>): void {
  commands.addCommand(CommandIDs.close, {
    execute: (args: any) => {
      const model = args.mindstudio as MindStudio.IModel;
      tracker.forEach((widget: MainAreaWidget<MindStudioReactWidget>) => {
        if (widget.content.currentMindStudioModel && widget.content.currentMindStudioModel.name === model.name) {
          widget.dispose();
          widget.close();
        }
      });
    },
  });
}

function addCreateNewCommand(commands: any, app: JupyterFrontEnd): void {
  commands.addCommand(CommandIDs.createNew, {
    label: (args: any) => (args.isPalette ? 'New MindStudio Insight' : 'MindStudio Insight'),
    caption: 'Start a new mindstudio insight',
    icon: (args: any) => (args.isPalette ? undefined : mindstudioIcon),
    execute: (args: any) => {
      // Try to open the session panel to make it easier for users to observe more active mindstudio instances
      try {
        app.shell.activateById('jp-running-sessions');
        app.commands.execute(CommandIDs.open);
      } catch (e) {
        showDialog({
          title: 'Cannot create mindstudio insight.',
          body: 'Something wrong!',
          buttons: [Dialog.okButton()],
        });
      }
    },
  });
}

function addRunningSessionManager(
  runningSessionManagers: IRunningSessionManagers,
  app: JupyterFrontEnd,
  manager: MindStudioManager
): Promise<void> {
  class RunningMindStudio implements IRunningSessions.IRunningItem {
    private _model: MindStudio.IModel;
    private _runningManager: MindStudioManager;

    constructor(model: MindStudio.IModel, runningManager: MindStudioManager) {
      this._model = model;
      this._runningManager = runningManager;
    }

    open(): void {
      app.commands.execute(CommandIDs.open, { modelName: this._model.name });
    }

    icon(): LabIcon {
      return mindstudioIcon;
    }

    label(): string {
      return `${this._model.name}`;
    }

    shutdown(): Promise<void> {
      app.commands.execute(CommandIDs.close, { tb: this._model });
      return this._runningManager.shutdown(this._model.name);
    }
  }

  return manager.getStaticConfigPromise.then(() => {
    runningSessionManagers.add({
      name: 'MindStudio Insight',
      running: () =>
        manager.running().map(model => new RunningMindStudio(model, manager)),
      shutdownAll: () => manager.shutdownAll(),
      refreshRunning: () => manager.refreshRunning(),
      runningChanged: manager.runningChanged,
    });
  });
}

