const autoKey = Symbol('autokey');

export type AutoKey<T extends object> = T & { [autoKey]?: string };

export const getAutoKey = (function<T extends object>() {
    let counter = 0;
    const generateRowKey = (): string => {
        if (counter === Number.MAX_SAFE_INTEGER) {
            counter = 0;
        }
        return `${counter++}`;
    };
    return (data: AutoKey<T>) => {
        let key = data[autoKey];
        if (key === undefined) {
            key = data[autoKey] = generateRowKey();
        }
        return key;
    };
})();
