/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 */
import { makeAutoObservable } from 'mobx';
import type { communicator, communicatorContainerData, ppData } from '../components/communicatorContainer/ContainerUtils';
export class Session {
    language: 'zhCN' | 'enUS' = 'enUS';
    parseCompleted: boolean = false;
    clusterCompleted: boolean = false;
    durationFileCompleted: boolean = false;
    unitcount: number = 0;
    renderId: number = 1;
    isFullDb: boolean = false;
    id = '';

    allRankIds: any[] = [];

    // communicatorContainerData
    communicatorData: communicatorContainerData;
    activeCommunicator: communicator | undefined;
    ranksData: ppData[] = [];
    rankCount: number = 0;

    constructor(conf?: Partial<Session>) {
        makeAutoObservable(this);
        this.communicatorData = {
            partitionModes: [],
            defaultPPSize: 0,
        };
        window.closeWaiting = (): void => {
            this.clusterCompleted = true;
            this.parseCompleted = true;
        };
    }
}
