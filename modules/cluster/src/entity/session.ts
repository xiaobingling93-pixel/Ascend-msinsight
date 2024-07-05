import { makeAutoObservable } from 'mobx';
import { communicator, communicatorContainerData } from '../components/communicatorContainer/ContainerUtils';
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
