/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */
export type Key = React.Key;

export type FixedType = 'left' | 'right' | boolean;

export type SvgType = React.FunctionComponent<React.SVGProps<
    SVGSVGElement
> & { title?: string }> & React.ReactNode;
