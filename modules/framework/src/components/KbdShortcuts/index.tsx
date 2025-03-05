/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2025-2025. All rights reserved.
 */

import React from 'react';
import { Modal } from 'antd';
import { useTranslation } from 'react-i18next';
import { KEYS, getShortcutKey } from 'ascend-utils';
import styled from '@emotion/styled';

const DELIMITER = '+';

const StyledModal = styled(Modal)`
    .ShortcutModal {
        color: ${(p): string => p.theme.textColor};

        &__modules-container {
            display: grid;
            grid-column-gap: 24px;
            grid-row-gap: 32px;
        }

        &__module {
            h4 {
                font-size: 14px;
                font-weight: 600;
                margin: 0;
                margin-bottom: 10px;
                color: ${(p): string => p.theme.textColor};
            }

            &-content {
                border: 1px solid ${(p): string => p.theme.borderColor};
                border-radius: 4px;
            }
        }

        &__shortcut {
            border-bottom: 1px solid  ${(p): string => p.theme.borderColor};
            padding: 6px 12px;
            display: flex;
            justify-content: space-between;
            align-items: center;
            font-size: 14px;
            column-gap: 8px;

            &:last-child {
                border-bottom: none;
            }
        }

        &__key-container {
            display: flex;
            align-items: center;
            column-gap: 4px;
            flex-shrink: 0;
        }

        &__key {
            display: flex;
            box-sizing: border-box;
            font-size: 12px;
            background-color: ${(p): string => p.theme.primaryColorLight2};
            border-radius: 4px;
            padding: 8px;
            word-break: keep-all;
            align-items: center;
            line-height: 1;
        }
    }
`;

const Section = (props: { title?: string; children: React.ReactNode }): JSX.Element => (
    <>
        {props.title !== undefined && <h3>{props.title}</h3>}
        <div className="ShortcutModal__modules-container">{props.children}</div>
    </>
);

const ShortcutModule = (props: {
    caption: string;
    children: React.ReactNode;
}): JSX.Element => (
    <div className={'ShortcutModal__module'}>
        <h4 className="ShortcutModal__module-title">{props.caption}</h4>
        <div className="ShortcutModal__module-content">{props.children}</div>
    </div>
);

const upperCaseSingleChars = (str: string): string => {
    return str.replace(/\b[a-z]\b/, (c) => c.toUpperCase());
};

function * intersperse(as: JSX.Element[][], delim: string | null): Generator<string | null | JSX.Element[]> {
    let first = true;
    for (const x of as) {
        if (!first) {
            yield delim;
        }
        first = false;
        yield x;
    }
}

const Shortcut = ({
    label,
    shortcuts,
}: {
    label: string;
    shortcuts: string[];
}): JSX.Element => {
    const { t } = useTranslation('framework');
    const splitShortcutKeys = shortcuts.map((shortcut) => {
        const keys = shortcut.split(DELIMITER);

        return keys.map((key) => (
            <ShortcutKey key={key}>{upperCaseSingleChars(key)}</ShortcutKey>
        ));
    });

    return (
        <div className="ShortcutModal__shortcut">
            <div>{label}</div>
            <div className="ShortcutModal__key-container">
                {intersperse(splitShortcutKeys, t('Or'))}
            </div>
        </div>
    );
};

const ShortcutKey = (props: { children: React.ReactNode }): JSX.Element => (
    <kbd className="ShortcutModal__key" {...props} />
);

export const ShortcutsModal = ({ open, onClose }: {open: boolean;onClose: () => void}): JSX.Element => {
    const { t } = useTranslation('framework');

    return <StyledModal
        title={t('Keyboard shortcuts')}
        open={open}
        onCancel={onClose}
        destroyOnClose={true}
        footer={null}
        width={'90%'}
        style={{ maxWidth: 800 }}
        wrapClassName="ShortcutModal"
    >
        <Section>
            <ShortcutModule caption={t('tabs.Timeline')}>
                <Shortcut
                    label={t('shortcuts.Zoom in')}
                    shortcuts={[KEYS.W]}
                />
                <Shortcut
                    label={t('shortcuts.Zoom out')}
                    shortcuts={[KEYS.S]}
                />
                <Shortcut
                    label={t('shortcuts.Pan left')}
                    shortcuts={[KEYS.A, `${getShortcutKey('CtrlOrCmd')}${DELIMITER}${t('shortcuts.drag')}`]}
                />
                <Shortcut
                    label={t('shortcuts.Pan right')}
                    shortcuts={[KEYS.D]}
                />
                <Shortcut
                    label={t('shortcuts.Toggle bottom drawer')}
                    shortcuts={[KEYS.Q]}
                />
                <Shortcut
                    label={t('shortcuts.M selection')}
                    shortcuts={[KEYS.M]}
                />
                <Shortcut
                    label={t('shortcuts.Zoom into a selection')}
                    shortcuts={[`${getShortcutKey('Alt')}${DELIMITER}${t('shortcuts.click')}`]}
                />
                <Shortcut
                    label={t('shortcuts.L align')}
                    shortcuts={[KEYS.L]}
                />
                <Shortcut
                    label={t('shortcuts.R align')}
                    shortcuts={[KEYS.R]}
                />
            </ShortcutModule>
        </Section>
    </StyledModal>;
};
