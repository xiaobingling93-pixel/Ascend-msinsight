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
import React from 'react';
import styled from '@emotion/styled';
import type { Theme } from '@emotion/react';

type Tone = 'default' | 'primary' | 'info' | 'success' | 'warning' | 'error';
type Depth = 1 | 2 | 3;

export interface TextProps extends React.HTMLAttributes<HTMLElement> {
    as?: keyof JSX.IntrinsicElements;
    type?: Tone;
    depth?: Depth;
    strong?: boolean;
    italic?: boolean;
    underline?: boolean;
    delete?: boolean;
    code?: boolean;
    ellipsis?: boolean; // 单行省略
    lineClamp?: number; // 多行省略
    disabled?: boolean;
    children?: React.ReactNode;
}

function resolveColor(theme: Theme, tone: Tone, depth?: Depth, disabled?: boolean): string {
    if (disabled) return theme.textColorDisabled;
    switch (tone) {
        case 'primary': return theme.primaryColor;
        case 'info': return theme.infoColor;
        case 'success': return theme.successColor;
        case 'warning': return theme.warningColor;
        case 'error': return theme.dangerColor;
        default:
            if (depth === 3) return theme.textColorTertiary;
            if (depth === 2) return theme.textColorSecondary;
            return theme.textColorPrimary;
    }
}

type StyledTextProps = Required<Pick<TextProps,
'type' | 'depth' | 'strong' | 'italic' | 'underline' | 'delete' | 'ellipsis' | 'lineClamp' | 'disabled'>>;
const StyledText = styled.span<StyledTextProps>(({
    theme,
    type,
    depth,
    strong,
    italic,
    underline,
    delete: del,
    ellipsis,
    lineClamp,
    disabled,
}) => ({
    color: resolveColor(theme, type, depth, disabled),
    fontWeight: strong ? 600 : undefined,
    fontStyle: italic ? 'italic' : undefined,
    textDecoration: [
        underline ? 'underline' : undefined,
        del ? 'line-through' : undefined,
    ].filter(Boolean).join(' ') || undefined,

    ...(ellipsis
        ? {
            overflow: 'hidden',
            whiteSpace: 'nowrap',
            textOverflow: 'ellipsis',
            display: 'inline-block',
            maxWidth: '100%',
        }
        : null),

    ...(lineClamp && lineClamp > 0
        ? {
            display: '-webkit-box',
            WebkitLineClamp: lineClamp,
            WebkitBoxOrient: 'vertical' as const,
            overflow: 'hidden',
        }
        : null),
}));

const Code = styled.code(({ theme }) => ({
    fontFamily: 'ui-monospace, SFMono-Regular, Menlo, Monaco, Consolas, "Liberation Mono", "Courier New", monospace',
    background: theme.bgColorCommon,
    border: `1px solid ${theme.borderColor}`,
    borderRadius: 6,
    padding: '0.125rem 0.375rem',
    fontSize: '0.9em',
}));

export const Text: React.FC<TextProps> = ({
    as,
    type = 'default',
    depth = 1,
    strong,
    italic,
    underline,
    delete: del,
    code,
    ellipsis,
    lineClamp,
    disabled,
    children,
    ...rest
}) => {
    let content = children;
    if (code) content = <Code>{content}</Code>;

    const Comp = (as ?? 'span') as any;

    return (
        <StyledText
            as={Comp}
            type={type}
            depth={depth}
            strong={!!strong}
            italic={!!italic}
            underline={!!underline}
            delete={!!del}
            ellipsis={!!ellipsis}
            lineClamp={lineClamp ?? 0}
            disabled={!!disabled}
            aria-disabled={disabled}
            {...rest}
        >
            {content}
        </StyledText>
    );
};
