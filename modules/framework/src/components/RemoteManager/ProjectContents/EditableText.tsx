/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */
import React, { useState, useRef, useEffect } from 'react';
import { Input } from 'ascend-components';
import styled from '@emotion/styled';
import type { InputRef } from 'antd';
import { updateProjectName } from '@/utils/Project';
import { HandleSingleDoubleClick } from 'ascend-utils';

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
}
function EditableText({ text = '' }: IProps): JSX.Element {
    const inputRef = useRef<InputRef>(null);
    const [editing, setEditing] = useState(false);
    const [editText, setEditText] = useState(text);
    // 双击进入编辑
    const handleDoubleClick = (): void => {
        // React不区分单击、双击,为避免单击事件运行，增加额外控制
        HandleSingleDoubleClick.doubleClick(() => {
            enterEditMode();
        }, 'projectName');
    };
    const enterEditMode = (): void => {
        setEditText(text);
        setEditing(true);
    };

    // 退出编辑
    const exitEdit = async (): Promise<void> => {
        if (editText !== '' && editText !== text) {
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
        <div className={editing ? 'hide' : 'show'} onDoubleClick={handleDoubleClick}>{ text }</div>
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
