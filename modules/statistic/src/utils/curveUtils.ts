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
import { useState, useEffect, useCallback } from 'react';
import { debounce } from 'lodash';

/**
 * 界面尺寸改变
 * @returns {readonly [number]}
 */
export function useResizeEventDependency(): readonly [number] {
    const [version, setVersion] = useState(0);

    const increaseSize = useCallback(
        debounce(() => {
            setVersion((prev) => prev + 1);
        }, 100),
        [],
    );

    useEffect(() => {
        window.addEventListener('resize', increaseSize);
        return () => {
            increaseSize.cancel();
            window.removeEventListener('resize', increaseSize);
        };
    }, []);

    return [version] as const;
};
