import { clamp } from 'lodash';
import { TimeStamp } from '../entity/common';
import { Time, TimeOptions, GetLength, adaptTime, getLastValue, isLowerUnit, GetPadder } from './adaptTimeForLength';

export type Readable = (inputs: number[]) => { value: number[]; unit: string };

const MEMUNIT = [
    'KB',
    'MB',
    'GB',
    'TB',
    'PB',
    'EB',
    'ZB',
    'YB',
    'BB',
];

const KILO = 1024;

/**
 * Output readable memory data, e.g. 204800KB => 200MB
 * @param inputKB numbers in KB
 * @returns numbers that readable
 */
export const getReadableMem: Readable = (inputKB) => {
    if (inputKB.length === 0) {
        return {
            value: [0],
            unit: MEMUNIT[0],
        };
    }
    let data = Math.max(...inputKB);
    let unitIndex = 0;
    const zoom = (): void => {
        if (data > KILO && unitIndex < (MEMUNIT.length - 1)) {
            data /= KILO;
            unitIndex += 1;
            zoom();
        }
    };
    zoom();
    return {
        value: inputKB.map(n => n / Math.pow(KILO, unitIndex)),
        unit: MEMUNIT[unitIndex],
    };
};

export function formatMemoryData(data: number): string {
    const item = getReadableMem([data]);
    return `${item.value[0].toFixed(2)} ${item.unit}`;
}

export const getTextualDuration = (endTimeAll: number | undefined): string => {
    if (endTimeAll === undefined) {
        return '00:00:00.000';
    }
    const time = endTimeAll;
    const hour = Math.floor(time / 3600000);
    const minute = Math.floor(time % 3600000 / 60000);
    const second = Math.floor(time % 60000 / 1000);
    const millisecond = time % 1000;
    let mesc = '';
    if (millisecond < 10) {
        mesc = `00${millisecond}`;
    } else if (millisecond < 100) {
        mesc = `0${millisecond}`;
    } else {
        mesc = `${millisecond}`;
    }
    return (hour < 10 ? `0${hour}` : `${hour}`) + ':' +
        (minute < 10 ? `0${minute}` : `${minute}`) + ':' +
        (second < 10 ? `0${second}` : `${second}`) + '.' +
        mesc;
};

/**
 * Converts a fixed-point number to percent string.
 *
 * @param numPercent the number to be displayed as a percent string
 * @param digits the number of digits to appear after the decimal point, should be between 0 to 20 inclusive
 * @returns a percent string representation of the given number
 */
export const toPercent = (numPercent: number, digits?: number): string => {
    return `${(numPercent * 100).toFixed(digits)}%`;
};

const DEFAULT_DURATION_OPTIONS: TimeOptions = {
    precision: 'ns',
    maxChars: Number.MAX_SAFE_INTEGER,
};
export const getDuration = (time: TimeStamp, options: TimeOptions = DEFAULT_DURATION_OPTIONS): string => {
    const getLength: GetLength = (time) => {
        const timeLength = time?.length ?? 0;
        const timeUnitLength = time?.unit.length ?? 0;
        return timeLength + timeUnitLength + 1;
    };
    const getPadder: GetPadder = (splitTime, emptyPadder, padStartIdx, timesIdx, length) => {
        if (options?.segments !== undefined) {
            padStartIdx !== 0 && splitTime.unshift(...emptyPadder.slice(-padStartIdx).map(item => item));
        } else {
            do {
                emptyPadder[padStartIdx] !== undefined && splitTime.unshift(emptyPadder[padStartIdx]);
                (length as number) += getLength(emptyPadder[padStartIdx]);
                padStartIdx--;
            } while ((length as number) < (options.maxChars as number) - 2 && padStartIdx >= 0);
        }
        return [ splitTime, timesIdx ];
    };
    const { splitTime: duration, tail } = adaptTime(time, { getLength, getPadder, ...options });
    const removeInvalidNum = (times: Time[]): Time[] => {
        for (let i = 0; i < times.length; i++) {
            if (times[i].value !== 0 || i === times.length - 1) {
                return times.slice(i);
            }
        }
        return times;
    };
    const validTime = removeInvalidNum(duration);
    return validTime.map((item, index) => `${getLastValue(index === validTime.length - 1, item.value, tail)}${item.unit}`).join(' ');
};

const DEFAULT_TIMESTAMP_OPTIONS: TimeOptions = {
    precision: 'ms',
    segments: 6,
};

const getLength = (time?: Time): number => {
    let padder = 2;
    if (time?.unit === 'ms') {
        padder = 3;
    }
    const length = time?.length ?? 0;
    const padderLength = time?.value === 0 ? padder : 0;
    return length + Math.max(0, padderLength - length);
};

export const getTimestamp = (time: TimeStamp, options: TimeOptions = DEFAULT_TIMESTAMP_OPTIONS): string => {
    const sign = time < 0 ? '-' : '';
    time = Math.abs(time);
    const mode = options?.maxChars === undefined
        ? { segments: 6, ...options }
        : options;
    const getPadder: GetPadder = (splitTime, emptyPadder, padStartIdx, timesIdx, length) => {
        let resTime: Time[] = [];
        if (mode?.segments !== undefined) {
            emptyPadder = emptyPadder.reverse();
            for (let i = 0; i <= padStartIdx || isLowerUnit(splitTime[0]) || splitTime[0]?.unit === 's'; i++) {
                emptyPadder[i] !== undefined && splitTime.unshift(emptyPadder[i]);
            }
            resTime = splitTime.slice(0, mode.segments);
            timesIdx = clamp(splitTime.length - resTime.length, 0, splitTime.length) - timesIdx;
        } else {
            do {
                emptyPadder[padStartIdx] !== undefined && splitTime.unshift(emptyPadder[padStartIdx]);
                (length as number) += getLength(emptyPadder[padStartIdx]);
                padStartIdx--;
            } while ((length as number) < (mode.maxChars as number) - 2 && padStartIdx >= 0);
            resTime = splitTime;
        }
        return [ resTime, timesIdx ];
    };
    const { splitTime: timestamp } = adaptTime(time, { getLength, getPadder, ...mode });
    let separator: ':' | '.' = ':';
    let maxLength = 2;
    return sign + timestamp.reduce((acc, item, index) => {
        if (isLowerUnit(item)) {
            maxLength = 3;
            separator = '.';
        }
        const isInvalidHour = item.unit === 'hour' && item.value === 0;
        const splitter = index === 0 || isInvalidHour ? '' : separator;
        const res = isInvalidHour
            ? ''
            : acc += `${splitter}${item.value.toString().padStart(maxLength, '0')}`;
        return res;
    }, '');
};

/**
 * toLocalTimeString
 * @param timestamp timestamp
 */
export function toLocalTimeString(timestamp: number): string {
    const date = new Date(timestamp);
    return date.toLocaleDateString('zh', { hour12: false }).replaceAll('/', '-') +
        ' ' +
        date.toLocaleTimeString('zh', { hour12: false }).replaceAll('/', '-');
}

/**
 * Converts a number to thousand separated form. eg: 12345678 => 12,345,678
 */
export function thousandSeparated(n: number): string {
    return Math.floor(n).toLocaleString('en-US');
}

/**
 * Transform camel-case field to frontend readable string
 * @param camelCaseStr eg: sampleString
 * @returns eg: Sample String
 */
export function transformCamelCaseToReadable(camelCaseStr: string): string {
    if (camelCaseStr.length < 2 || !/^[a-zA-Z]+$/.test(camelCaseStr)) {
        throw new Error('Invalid camel case string for func transformCamelCaseToReadable()! Input string must be pure letters whose length is over 2.');
    }
    const initialUpperStr = camelCaseStr[0].toUpperCase() + camelCaseStr.substring(1);
    const pattern = /[A-Z][a-z]+/g;
    const readableStr = initialUpperStr.match(pattern);
    return readableStr?.join(' ') as string;
}

export const nsToMs = (numPercent: number, digits?: number): string => {
    return `${(numPercent / 1e6).toFixed(digits)}`;
};

export function formatDigit(input: number): string {
    if (input >= 10) {
        return input.toFixed(2);
    } else {
        return input.toPrecision(4);
    }
}
