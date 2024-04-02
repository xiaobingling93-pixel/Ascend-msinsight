import { makeAutoObservable } from 'mobx';
import { communicator, communicatorContainerData } from '../components/communicatorContainer/ContainerUtils';
export class Session {
    clusterStatus: boolean = false;
    parseCompleted: boolean = false;
    clusterCompleted: boolean = true;
    durationFileCompleted: boolean = false;
    unitcount: number = 0;
    renderId: number = 1;
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
        window.closeWaiting = () => {
            this.clusterCompleted = true;
            this.parseCompleted = true;
        };
    }
}
