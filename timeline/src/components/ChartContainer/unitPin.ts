const pinState = Symbol('pinnedState');
const sonPinned = Symbol('sonPinned');

export type PinUnit<T extends object> = T & { [pinState]?: boolean; children?: Array<PinUnit<T>>; [sonPinned]?: boolean};

export const isPinned = function<T extends object>(unit: PinUnit<T>): boolean {
    if (unit[pinState] === undefined) {
        unit[pinState] = false;
    }
    return unit[pinState];
};

export const isSonPinned = function <T extends object>(unit: PinUnit<T>): boolean {
    if (unit[sonPinned] === undefined) {
        unit[sonPinned] = false;
    }
    return unit[sonPinned];
};

export const switchPinned = function<T extends object>(unit: PinUnit<T>): void {
    if (unit[pinState] === undefined) {
        unit[pinState] = false;
    }
    unit[pinState] = !unit[pinState];
    switchSonPinned(unit);
};

function switchSonPinned<T extends object>(unit: PinUnit<T>): void {
    if (unit.children) {
        unit.children.forEach(element => {
            element[sonPinned] = unit[pinState];
        });
    }
}
