/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
*/
import styled from '@emotion/styled';
import { Descriptions } from 'antd';

export const MIDescriptions = styled(Descriptions)`
    color: ${(props): string => props.theme.textColorPrimary};
  
    .ant-descriptions-item-label {
      color: ${(props): string => props.theme.textColorTertiary};
    }
    .ant-descriptions-item-content {
      color: ${(props): string => props.theme.textColorPrimary};
    }
    .ant-descriptions-row>td, .ant-descriptions-row>th{
      padding-bottom: 10px;
    }
`;

export default MIDescriptions;
