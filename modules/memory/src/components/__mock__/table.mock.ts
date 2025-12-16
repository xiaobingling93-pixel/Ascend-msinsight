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

import { type MemoryTableColumn, type OperatorDetail } from '../../entity/memory';

export const tableData: { columns: MemoryTableColumn[]; rows: OperatorDetail[] } = {
    columns: [
        {
            name: 'Name',
            type: 'string',
            key: 'name',
        },
        {
            name: 'Size(KB)',
            type: 'number',
            key: 'size',
        },
        {
            name: 'Allocation Time(ms)',
            type: 'number',
            key: 'allocationTime',
        },
        {
            name: 'Release Time(ms)',
            type: 'number',
            key: 'releaseTime',
        },
        {
            name: 'Duration(ms)',
            type: 'number',
            key: 'duration',
        },
        {
            name: 'Active Release Time(ms)',
            type: 'number',
            key: 'activeReleaseTime',
        },
        {
            name: 'Active Duration(ms)',
            type: 'number',
            key: 'activeDuration',
        },
        {
            name: 'Allocation Total Allocated(MB)',
            type: 'number',
            key: 'allocationAllocated',
        },
        {
            name: 'Allocation Total Reserved(MB)',
            type: 'number',
            key: 'allocationReserved',
        },
        {
            name: 'Allocation Total Active(MB)',
            type: 'number',
            key: 'allocationActive',
        },
        {
            name: 'Release Total Allocated(MB)',
            type: 'number',
            key: 'releaseAllocated',
        },
        {
            name: 'Release Total Reserved(MB)',
            type: 'number',
            key: 'releaseReserved',
        },
        {
            name: 'Release Total Active(MB)',
            type: 'number',
            key: 'releaseActive',
        },
        {
            name: 'Stream',
            type: 'string',
            key: 'streamId',
        },
    ],
    rows: [
        {
            name: 'aten::empty_strided',
            size: 0.5,
            allocationTime: 38.85,
            releaseTime: 50.92,
            duration: 12.07,
            allocationAllocated: 4977.92236328125,
            allocationReserved: 31604,
            releaseAllocated: 4977.921875,
            releaseReserved: 31604,
            activeReleaseTime: '50.92',
            activeDuration: 12.06,
            allocationActive: 4977.92236328125,
            releaseActive: 4977.921875,
            streamId: '187651642265632',
        },
        {
            name: 'aten::empty_strided',
            size: 513,
            allocationTime: 51.56,
            releaseTime: 732.96,
            duration: 681.4,
            allocationAllocated: 4978.4228515625,
            allocationReserved: 31604,
            releaseAllocated: 24645.70947265625,
            releaseReserved: 31604,
            activeReleaseTime: '732.95',
            activeDuration: 681.4,
            allocationActive: 4978.4228515625,
            releaseActive: 24645.70947265625,
            streamId: '187651642265632',
        },
        {
            name: 'aten::empty_strided',
            size: 0.5,
            allocationTime: 52.21,
            releaseTime: 53.19,
            duration: 0.98,
            allocationAllocated: 4978.42333984375,
            allocationReserved: 31604,
            releaseAllocated: 4978.4228515625,
            releaseReserved: 31604,
            activeReleaseTime: '53.19',
            activeDuration: 0.98,
            allocationActive: 4978.42333984375,
            releaseActive: 4978.4228515625,
            streamId: '187651642265632',
        },
        {
            name: 'aten::empty_strided',
            size: 150528.5,
            allocationTime: 116.7,
            releaseTime: 731.46,
            duration: 614.76,
            allocationAllocated: 5125.42333984375,
            allocationReserved: 31604,
            releaseAllocated: 24646.20849609375,
            releaseReserved: 31604,
            activeReleaseTime: '731.46',
            activeDuration: 614.76,
            allocationActive: 5125.42333984375,
            releaseActive: 24646.20849609375,
            streamId: '187651642265632',
        },
        {
            name: 'aten::clone',
            size: 256.5,
            allocationTime: 129.65,
            releaseTime: 731.42,
            duration: 601.76,
            allocationAllocated: 5125.673828125,
            allocationReserved: 31604,
            releaseAllocated: 24793.45947265625,
            releaseReserved: 31604,
            activeReleaseTime: '731.42',
            activeDuration: 601.76,
            allocationActive: 5125.673828125,
            releaseActive: 24793.45947265625,
            streamId: '187651642265632',
        },
    ],
};

export const tableConfig = {
    current: 1,
    pageSize: 10,
    total: 5,
    onCurrentChange: jest.fn(),
    onPageSizeChange: jest.fn(),
    onOrderChange: jest.fn(),
    onOrderByChange: jest.fn(),
};
