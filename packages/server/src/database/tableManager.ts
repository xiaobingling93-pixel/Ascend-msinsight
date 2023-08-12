/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 */

import { Table } from './table';
import { ClusterDatabase } from './cluster_database';
import { CommunicationAnalysisDataBase } from './communicationAnalysisDataBase';

// rankId, table object
export const tableMap = new Map<string, Table>();
export const CLUSTER_DATABASE = new ClusterDatabase('cluster.db');
export const communicationDbMap = new Map<string, CommunicationAnalysisDataBase>();
