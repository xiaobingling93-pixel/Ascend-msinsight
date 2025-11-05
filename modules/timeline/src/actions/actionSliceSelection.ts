import { register } from './register';
import type { Session } from '../entity/session';
import { runInAction } from 'mobx';

const toggleSelectMode = (session: Session): void => {
    runInAction(() => {
        session.sliceSelection.active = !session.sliceSelection.active;
        session.renderTrigger = !session.renderTrigger;
        session.sliceSelection.startPos = [];
        session.sliceSelection.activeIsChanged = true;
    });
};

export const actionSliceSelection = register({
    name: 'sliceSelection',
    label: 'timeline:contextMenu.SliceSelectionMode',
    checked: (session) => session.sliceSelection.active,
    perform: (session): void => {
        toggleSelectMode(session);
    },
});
