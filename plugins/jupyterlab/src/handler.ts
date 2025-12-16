/*
 * -------------------------------------------------------------------------
 * This file is part of the MindStudio project.
 * Copyright (c) 2025 Huawei Technologies Co.,Ltd.
 *
 * MindStudio is licensed under Mulan PSL v2.
 * You can use this software according to the terms and conditions of the Mulan PSL v2.
 * You may obtain a copy of Mulan PSL v2 at:
 *
 *          http://license.coscl.org.cn/MulanPSL2
 *
 * THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
 * EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
 * MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
 * See the Mulan PSL v2 for more details.
 * -------------------------------------------------------------------------
 */

import { URLExt } from '@jupyterlab/coreutils';
import { ServerConnection } from '@jupyterlab/services';

function sanitize(obj: any, loopIndex = 0): any {
  const MaxLoop = 1000;
  if (loopIndex > MaxLoop) {
      return;
  }
  if (obj && typeof obj === "object") {
    for (const key of Object.keys(obj)) {
      if (key === "__proto__" || key === "constructor" || key === "prototype") {
        delete obj[key];
      } else {
        sanitize(obj[key], loopIndex++);
      }
    }
  }
  return obj;
}

/**
 * Call the API extension
 *
 * @param endPoint API REST end point for the extension.
 * @param init Initial values for the request.
 * @returns The response body interpreted as JSON.
 */
export async function requestAPI<T>(
  endPoint = '',
  init: RequestInit = {}
): Promise<T> {
  // Make request to Jupyter API
  const settings = ServerConnection.makeSettings();
  // Set request url
  const requestUrl = URLExt.join(
    settings.baseUrl,
    'jupyterlab-mindstudio-insight', // API Namespace
    endPoint
  );

  // 获取server connection并校验
  let response: Response;
  try {
    response = await ServerConnection.makeRequest(requestUrl, init, settings);
  } catch (error) {
    throw new ServerConnection.NetworkError(error as TypeError);
  }
  
  // 获取后端返回json数据进行处理
  let data: any = await response.text();
  if (data.length > 0) {
    try {
      data = JSON.parse(data);
      data = sanitize(data);
    } catch (error) {
      data = {};
    }
  }

  // 抛出后端返回异常
  if (!response.ok) {
    throw new ServerConnection.ResponseError(response, data.message || data);
  }

  return data;
}
