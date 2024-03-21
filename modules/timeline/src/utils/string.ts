export function anonymousString(str: string): string {
    if (str === undefined || str.length < 3) {
        return str;
    }
    const pos = str.length / 3;
    const anonymousStr = '*'.repeat(pos);
    return str.substring(0, pos) + anonymousStr + str.substring(pos * 2, str.length);
}

export function handlerEmptyString(str: string, defaultValue: string): string {
    if (str === undefined || str.length === 0) {
        return defaultValue;
    }
    return str;
}
