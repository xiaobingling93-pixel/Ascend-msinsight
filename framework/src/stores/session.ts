import { defineStore } from "pinia";
import { reactive } from 'vue';
import { isConnected } from "@/centralServer/server";
import { LOCAL_HOST, PORT } from "@/centralServer/websocket/defs";
import connector from '@/connection/index';

export type Timestamp = number;
export class Session {
    language = 'enUS';
    startTime: Timestamp = -1;
    endTimeAll: Timestamp = -1;
    isCluster: boolean | null = false;
    isIpynb: boolean = false;
    ipynbUrl: string = '';
    isReset: boolean = false;
    parseCompleted: boolean = false;
    clusterCompleted: boolean = false;
    durationFileCompleted: boolean = false;
    loading: boolean = false;
    unitcount: number = 0;
    isFullDb: boolean = false;
    isVscode: boolean = document.location.origin.startsWith('vscode');
    // Compute
    isBinary: boolean | null = false;
    coreList: string[] = [];
    sourceList: string[] = [];
    memoryRankIds: string[] = [];
    operatorRankIds: string[] = [];
    private _sharedState: Record<string, unknown> = {};

    get sharedState(): Record<string, unknown> {
        return this._sharedState;
    }

    set sharedState(newState: Record<string, unknown>) {
        this._sharedState = newState;
        connector.send({
            event: 'updateSharedState',
            body: this._sharedState,
        });
    }

    get hasLocalServer(): boolean {
        return isConnected({ remote: LOCAL_HOST, port: PORT, projectName: '', dataPath: [] });
    }

    reset(remove = false): void {
        this.startTime = -1;
        this.endTimeAll = -1;
        this.isCluster = null;
        this.isIpynb = false;
        this.isReset = false;
        this._sharedState = {};
        this.parseCompleted = false;
        this.clusterCompleted = false;
        this.durationFileCompleted = false;
        this.unitcount = 0;
        this.isBinary = null;
        this.coreList = [];
        this.sourceList = [];
        if (remove === true) {
            this.isCluster = false;
            this.isBinary = false;
        }
        this.memoryRankIds = [];
        this.operatorRankIds = [];
    }
};

export const useSession = defineStore('session', () => {
    const session = reactive(new Session());
    const setSession = function<T extends keyof Session>(curState: Record<T, Session[T]> | ((prevState: typeof session) => Record<T, Session[T]>)): void {
        Object.assign(session, typeof curState !== 'function' ? curState : curState(session));
    }
    return { session, setSession };
});
