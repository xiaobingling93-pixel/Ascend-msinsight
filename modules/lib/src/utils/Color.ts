/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */
import { useTheme } from '@emotion/react';

const COLOR = {
    BRIGHT_BLUE: '#7df7ff',
    Grey20: '#cacaca',
    Grey40: '#989898',
    Grey50: '#7b7a7a',
    Band0: '#f82d18',
    Band1: '#eac299',
    Band2: '#c7eef5',
    Band3: '#0177ff',
    LIGHT_BLUE: '#1890ff',
};

export function useThemeColor(darkColor: string = '#D1D1D1', lightColor: string = '#595959'): string {
    const curTheme = useTheme().mode;
    if (curTheme === 'light') {
        return lightColor;
    } else {
        return darkColor;
    }
}

export function compareColors(color1: string, color2: string): boolean {
    // 将颜色值统一转换为 HEX 格式
    const toHex = (color: string): string => {
        // 如果是 HEX 格式，直接返回（去掉可能的 '#'）
        if (color.startsWith('#')) {
            return color.slice(1).toUpperCase();
        }

        // 如果是 RGB 格式，解析并转换为 HEX
        const rgbMatch = color.match(/\d+/g);
        if (!rgbMatch) return '';

        const r = parseInt(rgbMatch[0], 10);
        const g = parseInt(rgbMatch[1], 10);
        const b = parseInt(rgbMatch[2], 10);

        // 检查 RGB 值是否有效
        if (r < 0 || r > 255 || g < 0 || g > 255 || b < 0 || b > 255) {
            return '';
        }

        // 转换为 HEX 格式
        return `${r.toString(16).padStart(2, '0')}${g.toString(16).padStart(2, '0')}${b.toString(16).padStart(2, '0')}`.toUpperCase();
    };

    // 将两个颜色值转换为 HEX 格式
    const hex1 = toHex(color1);
    const hex2 = toHex(color2);

    // 如果转换失败，返回 false
    if (!hex1 || !hex2) {
        return false;
    }

    // 比较 HEX 值
    return hex1 === hex2;
}
export default COLOR;
