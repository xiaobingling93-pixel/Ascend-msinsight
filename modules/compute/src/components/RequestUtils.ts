/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
*/
/**
 * 查询源代码
 *
 * @param {sourceName} rankId RankID
 * @return {fileContent:string}
 */
export const querySourceCode = async(sourceName: string): Promise<any> => {
    return window.requestData('source/code/file', { sourceName });
};

/**
 * 查询代码行执行信息
 *
 * @param {sourceName} 源文件名
 * @param {coreName} 内核
 * @return {lines:[]}
 */
export const queryApiLine = async(param: {sourceName: string; coreName: string}): Promise<any> => {
    return window.requestData('source/api/line', param);
};

/**
 * 查询代码行执行信息
 *
 * @return {instructions:string}
 */
export const queryApiInstr = async(): Promise<any> => {
    return window.requestData('source/api/instructions', {});
};
