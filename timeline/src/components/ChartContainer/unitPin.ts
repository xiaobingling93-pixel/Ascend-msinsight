const pinState = Symbol('pinnedState');

export type PinUnit<T extends object> = T & { [pinState]?: boolean };

export const isPinned = function<T extends object>(unit: PinUnit<T>): boolean {
    if (unit[pinState] === undefined) {
        unit[pinState] = false;
    }
    return unit[pinState];
};

export const switchPinned = function<T extends object>(unit: PinUnit<T>): void {
    if (unit[pinState] === undefined) {
        unit[pinState] = false;
    }
    unit[pinState] = !unit[pinState];
};
