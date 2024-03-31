import { defineStore } from "pinia";
import { reactive } from 'vue';
import { isConnected } from "@/centralServer/server";
import { LOCAL_HOST, PORT } from "@/centralServer/websocket/defs";
import connector from '@/connection/index';

export type Timestamp = number;
export class Session {
    startTime: Timestamp = -1;
    endTimeAll: Timestamp = -1;
    isCluster: boolean = false;
    isReset: boolean = false;
    parseCompleted: boolean = false;
    clusterCompleted: boolean = false;
    durationFileCompleted: boolean = false;
    loading: boolean = false;
    unitcount: number = 0;
    isFullDb: boolean = false;
    isVscode: boolean = document.location.origin.startsWith('vscode');
    // Compute
    isBinary: boolean = false;
    coreList: string[] = [];
    sourceList: string[] = [];
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
        return isConnected({ remote: LOCAL_HOST, port: PORT, dataPath: [] });
    }

    reset(): void {
        this.startTime = -1;
        this.endTimeAll = -1;
        this.isCluster = false;
        this.isReset = false;
        this._sharedState = {};
        this.parseCompleted = false;
        this.clusterCompleted = false;
        this.durationFileCompleted = false;
        this.unitcount = 0;
        this.isBinary = false;
        this.coreList = [];
        this.sourceList = [];
    }
};

export const useSession = defineStore('session', () => {
    const session = reactive(new Session());
    const setSession = function<T extends keyof Session>(curState: Record<T, Session[T]> | ((prevState: typeof session) => Record<T, Session[T]>)) {
        Object.assign(session, typeof curState !== 'function' ? curState : curState(session));
    }
    return { session, setSession };
});
