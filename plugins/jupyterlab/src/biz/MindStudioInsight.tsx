/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */

import React, { useState, useEffect, useRef } from 'react';
import { MindStudioManager } from '../manager';
import * as MindStudio from '../mindstudio';

interface MindStudioInsightProps {
    setWidgetName?: (name: string) => void;
    createdModelName?: string;
    mindstudioManager: MindStudioManager;
    closeWidget: () => void;
    openMindStudio: (modelName: string) => void;
    updateCurrentModel: (model: MindStudio.IModel | null) => void;
    startNew: (name: string, options?: MindStudio.IOptions) => Promise<MindStudio.IMindStudio>;
}

export const MindStudioInsightTab = (props: MindStudioInsightProps): JSX.Element => {
    const [currentMindStudio, setCurrentMindStudio] = useState<MindStudio.IModel | null>(null);
    const currentMindStudioRef = useRef(currentMindStudio);
    // currently inactive
    const [notActiveError, setNotActiveError] = useState(false);
    const [srcVal, setSrcVal] = useState('');

    const updateCurrentMindStudio = (model: MindStudio.IModel | null): void => {
        props.updateCurrentModel(model);
        setCurrentMindStudio(model);
        currentMindStudioRef.current = model;
    };

    const refreshRunning = async (): Promise<void> => {
        await props.mindstudioManager.refreshRunning();
        const runningMindStudios = [...props.mindstudioManager.running()];

        // hint: Using runningMindStudios directly may cause setState to fail to respond
        const modelList = [];
        for (const model of runningMindStudios) {
            modelList.push(model);
        }

        if (currentMindStudioRef.current) {
            if (!modelList.find(model => model.name === currentMindStudioRef.current?.name)) {
                setNotActiveError(true);
            }
        }

        const model = props.createdModelName
            ? modelList.find(modelItem => modelItem.name === props.createdModelName)
            : null;
        if (model) {
            updateCurrentMindStudio(model);
            if (props.setWidgetName) {
                props.setWidgetName(model.name);
            }
        }
    };

    const getUrl = async (): Promise<void> => {
        const val = await MindStudio.getUrl('');
        setSrcVal(val);
    };

    useEffect(() => {
        refreshRunning();
        getUrl();
    }, []);

    return (
        <div style={{ width: '100%', height: '100%' }}>
            <div style={{ width: '100%', height: '100%' }}>
                { !currentMindStudio && (
                <iframe
                    style={{ width: '100%', height: '100%' }}
                    sandbox="allow-scripts allow-forms allow-same-origin"
                    referrerPolicy="no-referrer"
                    src={srcVal}
                />
                ) }
                { currentMindStudio && (
                <div>
                    <p>
                        No instance for current directory yet, please create a new MindStudio Insight.
                    </p>
                </div>
                ) }
                { notActiveError && (
                <div>
                    <p>
                    Current Tensorboard is not active. Please select others or create a new one.
                    </p>
                </div>
                ) }
            </div>
        </div>
    );
};