/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */
import React, { useState, useRef, useEffect } from 'react';
import { Input } from '@insight/lib/components';
import styled from '@emotion/styled';
import type { InputRef } from 'antd';
import { updateProjectName } from '@/utils/Project';
import { HandleSingleDoubleClick } from '@insight/lib/utils';
import type { Session } from '@/entity/session';
import { message } from 'antd';
import { useTranslation } from 'react-i18next';

const Container = styled.div`
  .show {
    overflow: hidden;
    white-space: nowrap;
    text-overflow: ellipsis;
  }
  .hide {
    display: none;
  }
  input {
    width: 100% ;
  }
`;

interface IProps {
    text: string;
    session?: Session;
    projectName?: string;
}
function EditableText({ text = '', session, projectName }: IProps): JSX.Element {
    const { t } = useTranslation('framework');
    const inputRef = useRef<InputRef>(null);
    const [editing, setEditing] = useState(false);
    const [editText, setEditText] = useState(text);
    // 双击进入编辑
    const handleDoubleClick = (): void => {
        let isSelectBaseline = false;
        if (session?.compareSet) {
            const { compareSet: { baseline, comparison } } = session;
            isSelectBaseline = baseline.filePath.startsWith(projectName as string) || comparison.filePath.startsWith(projectName as string);
        }
        if (!isSelectBaseline) {
            // React不区分单击、双击,为避免单击事件运行，增加额外控制
            HandleSingleDoubleClick.doubleClick(() => {
                enterEditMode();
            }, 'projectName');
        } else {
            HandleSingleDoubleClick.doubleClick(() => {
                message.warning(t('BaselineDataComparisonDataCannotRename'));
            }, 'projectName');
            // "基线数据和对比数据不允许重命名",
        }
    };
    const enterEditMode = (): void => {
        setEditText(text);
        setEditing(true);
    };

    // 退出编辑
    const exitEdit = async (): Promise<void> => {
        const trimmedContent = editText.trim();
        if (trimmedContent !== '' && trimmedContent !== text) {
            const success = await updateProjectName(text, editText);
            if (success) {
                setEditing(false);
            }
        } else {
            setEditing(false);
        }
    };

    const blurInput = (): void => {
        inputRef.current?.blur();
    };

    useEffect(() => {
        if (editing) {
            inputRef.current?.focus();
        }
    }, [editing]);

    return <Container>
        <div className={`can-right-click ${editing ? 'hide' : 'show'}`} onDoubleClick={handleDoubleClick}>{ text }</div>
        <Input className={editing ? 'show' : 'hide'}
            ref={inputRef}
            value={editText}
            onChange={(e): void => { setEditText(e.target.value); }}
            onPressEnter={blurInput}
            onBlur={exitEdit}
            onClick={(e): void => { e.stopPropagation(); }}
        />
    </Container>;
};

export default EditableText;
