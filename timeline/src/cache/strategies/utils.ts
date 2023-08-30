/**
 * binarySearch
 * @param arr data
 * @param getValue computes a value from a given data for comparison
 * @param target target
 * @returns index
 */
export function binarySearchFirstBig<T>(arr: T[], getValue: (elem: T) => number, target: number): number {
    if (arr.length <= 1) { return 0; }
    let lowIndex = 0;
    let highIndex = arr.length - 1;

    while (lowIndex <= highIndex) {
        const midIndex = Math.floor((lowIndex + highIndex) / 2);
        if (getValue(arr[midIndex]) >= target) {
            if (midIndex === 0 || getValue(arr[midIndex - 1]) < target) { return midIndex; }
            highIndex = midIndex - 1;
        } else {
            lowIndex = midIndex + 1;
        }
    }
    return arr.length - 1;
}

/**
 * binarySearch
 * @param arr data
 * @param getValue computes a value from a given data for comparison
 * @param target target
 * @returns index
 */
export function binarySearchLastSmall<T>(arr: T[], getValue: (elem: T) => number, target: number): number {
    if (arr.length <= 1) { return 0; }
    let lowIndex = 0;
    let highIndex = arr.length - 1;

    while (lowIndex <= highIndex) {
        const midIndex = Math.floor((lowIndex + highIndex) / 2);
        if (getValue(arr[midIndex]) <= target) {
            if (midIndex === arr.length - 1 || getValue(arr[midIndex + 1]) > target) { return midIndex; }
            lowIndex = midIndex + 1;
        } else {
            highIndex = midIndex - 1;
        }
    }
    return 0;
}

/**
 * binarySearchLastSmall function adaptation, processing basic data types.
 *
 * The same implementation function can jump to the getTime function in the TimeSeriesCache class,
 * which obtains a property value in the object.
 * @param e
 */
export function self<T>(e: T): T {
    return e;
}
