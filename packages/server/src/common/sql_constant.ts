/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 */

export const CLUSTER_BASE_INFO_TABLE = 'cluster_base_info';
export const COMMUNICATION_TIME_INFO_TABLE = 'communication_time_info';
export const COMMUNICATION_BAND_WIDTH_TABLE = 'communication_bandwidth_info';
export const STEP_STATISTIC_INFO_TABLE = 'step_statistic_info';
export const KERNEL_DETAIL_TABLE = 'kernel_detail';

export const CREATE_COMMUNICATION_TIME_INFO_SQL = `CREATE TABLE ${COMMUNICATION_TIME_INFO_TABLE}
                                               (
                                                   id                         INTEGER PRIMARY KEY AUTOINCREMENT,
                                                   iteration_id               INT,
                                                   rank_id                    INT,
                                                   op_name                    VARCHAR(100),
                                                   elapse_time                double,
                                                   synchronization_time_Ratio double,
                                                   synchronization_time       double,
                                                   Transit_Time               double,
                                                   wait_time_ratio            double,
                                                   wait_time                  double
                                               )`;
export const CREATE_COMMUNICATION_BANDWIDTH_INFO_SQL = `CREATE TABLE ${COMMUNICATION_BAND_WIDTH_TABLE}
                                                    (
                                                        id                    INTEGER PRIMARY KEY AUTOINCREMENT,
                                                        iteration_id          INT,
                                                        rank_id               INT,
                                                        op_name               VARCHAR(100),
                                                        transport_type        VARCHAR(20),
                                                        bandwidth_size        double,
                                                        bandwidth_utilization double,
                                                        large_package_ratio   double,
                                                        size_distribution     json,
                                                        transit_size          double,
                                                        transit_time          double
                                                    )`;

export const CREATE_CLUSTER_TABLE_SQL = `CREATE TABLE ${CLUSTER_BASE_INFO_TABLE}
                                      (
                                          id                 INTEGER PRIMARY KEY AUTOINCREMENT,
                                          file_path          VARCHAR(500),
                                          ranks         json,
                                          steps         json,
                                          collect_start_time DATETIME,
                                          collect_duration   double,
                                          data_size          double
                                      )`;
export const CREATE_STEP_STATISTIC_INFO_TABLE_SQL = `CREATE TABLE ${STEP_STATISTIC_INFO_TABLE}
                                                (
                                                    id                                      INTEGER PRIMARY KEY AUTOINCREMENT,
                                                    rank_id                                 VARCHAR(50),
                                                    step_id                                 VARCHAR(50),
                                                    stage_id                                VARCHAR(50),
                                                    compute_time                            double,
                                                    pure_communication_time                 double,
                                                    overlap_communication_time              double,
                                                    communication_time                      double,
                                                    free_time                               double,
                                                    stage_time                              double,
                                                    bubble_time                             double,
                                                    pure_communication_exclude_receive_time double
                                                )`;

export const CREATE_KERNEL_DETAIL_TABLE_SQL = `CREATE TABLE ${KERNEL_DETAIL_TABLE}
                                           (
                                               id                INTEGER PRIMARY KEY AUTOINCREMENT,
                                               name              VARCHAR(50),
                                               type              VARCHAR(50),
                                               accelerator_core  VARCHAR(50),
                                               start_time        DATETIME,
                                               duration          double,
                                               wait_time         double,
                                               block_dim         double,
                                               input_shapes      VARCHAR(300),
                                               input_data_types  VARCHAR(200),
                                               input_formats     VARCHAR(200),
                                               output_shapes     VARCHAR(300),
                                               output_data_types VARCHAR(200),
                                               output_formats    VARCHAR(200)
                                           )`;
