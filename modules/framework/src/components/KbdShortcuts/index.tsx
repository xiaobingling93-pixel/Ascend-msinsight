/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2025-2025. All rights reserved.
 */

import React, { useEffect, useState } from 'react';
import { Modal } from 'antd';
import { useTranslation } from 'react-i18next';
import { KEYS, getShortcutKey } from '@insight/lib/utils';
import styled from '@emotion/styled';
import { Tooltip } from '@insight/lib/components';
import { HelpIcon } from '@insight/lib/icon';

const DELIMITER = '+';

const StyledModal = styled(Modal)`
    .ShortcutModal {
        color: ${(p): string => p.theme.textColor};

        &__header {
            display: flex;
            align-items: center;
            padding: 6px 12px;
            margin-bottom: 16px;
            border: 1px solid ${(p): string => p.theme.borderColor};
            border-radius: 4px;

            .name {
                display: flex;
                align-items: center;
                gap: 6px;
                font-weight: bold;
            }

            .value {
                font-style: italic;
            }
        }

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

const MAX_SIZE = 10 * 1024;
const FILE_URL = 'public-URL.json';
async function fetchJSON(url: string): Promise<any> {
    const response = await fetch(url, { method: 'HEAD' });
    const contentLength = response.headers.get('Content-Length');
    if (contentLength && Number(contentLength) > MAX_SIZE) {
        throw new Error('文件过大，已拒绝加载');
    }

    const dataResponse = await fetch(url);
    return dataResponse.json();
}
const Header = (): JSX.Element => {
    const { t } = useTranslation('framework');
    const [url, setUrl] = useState('');

    useEffect(() => {
        fetchJSON(FILE_URL)
            .then(data => setUrl(data.documentation))
            .catch((error) => {
                throw new Error(`JSON 解析失败：${(error as Error).message}`);
            });
    }, []);

    return url
        ? <div className="ShortcutModal__header">
            <div className="name">
                <Tooltip
                    title={<img src="images/documentation.png" alt="Documentation"/>}>
                    <HelpIcon style={{ cursor: 'pointer' }} height={20} width={20}/>
                </Tooltip>
                <div>{t('Documentation')}：</div>
            </div>
            <div className="value">{url}</div>
        </div>
        : <></>;
};

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
        <Header />
        <Section>
            <ShortcutModule caption={t('tabs.Timeline')}>
                <Shortcut
                    label={t('shortcuts.Zoom in')}
                    shortcuts={[KEYS.W, `${getShortcutKey('CtrlOrCmd')}${DELIMITER}${t('shortcuts.ScrollUp')}`]}
                />
                <Shortcut
                    label={t('shortcuts.Zoom out')}
                    shortcuts={[KEYS.S, `${getShortcutKey('CtrlOrCmd')}${DELIMITER}${t('shortcuts.ScrollDown')}`]}
                />
                <Shortcut
                    label={t('shortcuts.Undo zoom')}
                    shortcuts={[KEYS.BACKSPACE]}
                />
                <Shortcut
                    label={t('shortcuts.Reset zoom')}
                    shortcuts={[`${getShortcutKey('CtrlOrCmd')}+0`]}
                />
                <Shortcut
                    label={t('shortcuts.Zoom into selection')}
                    shortcuts={['Shift+Z']}
                />
                <Shortcut
                    label={t('shortcuts.Pan left')}
                    shortcuts={[KEYS.A, `${getShortcutKey('CtrlOrCmd')}${DELIMITER}${t('shortcuts.drag')}`]}
                />
                <Shortcut
                    label={t('shortcuts.Pan right')}
                    shortcuts={[KEYS.D, `${getShortcutKey('CtrlOrCmd')}${DELIMITER}${t('shortcuts.drag')}`]}
                />
                <Shortcut
                    label={t('shortcuts.Scroll up')}
                    shortcuts={['↑']}
                />
                <Shortcut
                    label={t('shortcuts.Scroll down')}
                    shortcuts={['↓']}
                />
                <Shortcut
                    label={t('shortcuts.Toggle bottom drawer')}
                    shortcuts={[KEYS.Q]}
                />
                <Shortcut
                    label={t('shortcuts.Create Flag Mark')}
                    shortcuts={[KEYS.K]}
                />
                <Shortcut
                    label={t('shortcuts.M selection')}
                    shortcuts={[KEYS.M]}
                />
                <Shortcut
                    label={t('shortcuts.Zoom into a selection')}
                    shortcuts={[`${getShortcutKey('Alt')}${DELIMITER}${t('shortcuts.drag')}`]}
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
            <ShortcutModule caption={t('tabs.ZoomEcharts')}>
                <Shortcut
                    label={`${t('shortcuts.Pan left')}/${t('shortcuts.Pan right')}`}
                    shortcuts={[`${getShortcutKey('Shift')}${DELIMITER}${t('shortcuts.Scorll')}`]}
                />
                <Shortcut
                    label={t('shortcuts.Zoom in/out')}
                    shortcuts={[`${getShortcutKey('CtrlOrCmd')}${DELIMITER}${t('shortcuts.Scorll')}`]}
                />
            </ShortcutModule>
            <ShortcutModule caption={t('tabs.Source')}>
                <Shortcut
                    label={t('shortcuts.Find in source code')}
                    shortcuts={[`${getShortcutKey('CtrlOrCmd')}${DELIMITER}F`]}
                />

            </ShortcutModule>
        </Section>
    </StyledModal>;
};
