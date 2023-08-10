/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 */

import { Table } from './table';
import { ClusterDatabase } from './clusterDatabase';
import { CommunicationAnalysisDataBase } from './communicationAnalysisDataBase';

// rankId, table object
export const tableMap = new Map<string, Table>();
export const clusterDatabase = new ClusterDatabase('cluster');
export const communicationDbMap = new Map<string, CommunicationAnalysisDataBase>();
