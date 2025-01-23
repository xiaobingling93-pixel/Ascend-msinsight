/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */
import React from 'react';
import { observer } from 'mobx-react';
import { Popconfirm, Tooltip } from 'antd';
import { DeleteIcon } from 'ascend-icon';
import { removeDataPath, removeProject } from '@/utils/Project';

interface IProps {
    isProject: boolean;
    projectIndex: number;
    dataPathIndex?: number;
}
const DeleteConfirm = observer(({ isProject, projectIndex, dataPathIndex }: IProps) => {
    const confirm = (): void => {
        if (isProject) {
            removeProject(projectIndex);
        } else {
            if (dataPathIndex !== undefined) {
                removeDataPath(projectIndex, dataPathIndex);
            }
        }
    };
    return <Popconfirm placement="topLeft"
        title={isProject ? 'DeleteProjectConfirmDescribe' : 'DeleteItemConfirmDescribe'}
        onConfirm={confirm}
        okText="Yes"
        cancelText="No"
        destroyTooltipOnHide={{ keepParent: false }}
    >
        <Tooltip placement="top" title="Delete" destroyTooltipOnHide={{ keepParent: false }}>
            <DeleteIcon/>
        </Tooltip>
    </Popconfirm>;
});

export default DeleteConfirm;
