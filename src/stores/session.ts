import { defineStore } from "pinia";
import { reactive } from 'vue';

export type Timestamp = number;
export class Session {
    startTime: Timestamp = -1;
    endTimeAll: Timestamp = -1; 
    isCluster: boolean = false;
    hasLocalServer: boolean = false;
};

export const useSession = defineStore('session', () => {
    const session = reactive(new Session());
    const setSession = function<T extends keyof Session>(curState: Record<T, Session[T]> | ((prevState: Session) => Record<T, Session[T]>)) {
        Object.assign(session, typeof curState !== 'function' ? curState : curState(session));
    }
    return { session, setSession };
});
