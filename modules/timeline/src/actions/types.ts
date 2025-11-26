/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2025-2025. All rights reserved.
 */

import type React from 'react';
import { Session } from '../entity/session';
import { InteractorMouseState } from '../components/charts/ChartInteractor/ChartInteractor';
import type { TFunction } from 'i18next';
import { ContextMenuItem } from '../components/ContextMenu';

type ActionFn = (
    session: Session,
    interactorMouseState?: InteractorMouseState,
    xScale?: (x: number) => number
) => void;

export type ActionName =
  | 'zoomIn'
  | 'zoomOut'
  | 'undoZoom'
  | 'resetZoom'
  | 'panLeft'
  | 'panRight'
  | 'scrollUp'
  | 'scrollDown'
  | 'toggleOperatorSelection'
  | 'alignToBenchmarkLeft'
  | 'alignToBenchmarkRight'
  | 'fitToScreen'
  | 'findInCommunication'
  | 'zoomIntoSelection'
  | 'unpinAll'
  | 'pinByGroupNameValue'
  | 'unpinByGroupNameValue'
  | 'parseCardsOfRelatedGroup'
  | 'hideUnits'
  | 'showHiddenUnits'
  | 'showInEventsView'
  | 'showPythonCallStack'
  | 'hidePythonCallStack'
  | 'collapseAllUnits'
  | 'expandAllUnits'
  | 'hideFlagEvents'
  | 'showFlagEvents'
  | 'timeRangeAnalysis'
  | 'removeTimeRangeAnalysis'
  | 'timeRangeAnalysisAndZoomIn'
  | 'applyTimeRangeAnalysis'
  | 'enableAutoUnitHeight'
  | 'disableAutoUnitHeight'
  | 'recoverDefaultOffset'
  | 'setBaseSlice'
  | 'clearBaseSlice'
  | 'toggleSelection'
  | 'toggleBottomPanel'
  | 'lockSelection'
  | 'unlockSelection'
  | 'setCardAlias'
  | 'mergeUnits'
  | 'unmergeUnits'
  | 'generateCurveByBlock'
  | 'generateBubbleCurveByBlock'
  | 'createFlagMark'
  | 'sliceSelection'
  | 'jumpToLinkSlice';

export interface Action {
    name: ActionName;
    label: string | ((session: Session, t: TFunction) => string);
    disabled?: (session: Session) => boolean;
    visible?: (session: Session) => boolean;
    perform: ActionFn;
    keyTest?: (
        event: React.KeyboardEvent | KeyboardEvent,
    ) => boolean;
    once?: boolean;
    checked?: (session: Session) => boolean;
    subMode?: boolean;
    subMenus?: (session: Session) => ContextMenuItem[];
    parentMenuKey?: string;
    style?: { [key: string]: any };
}
