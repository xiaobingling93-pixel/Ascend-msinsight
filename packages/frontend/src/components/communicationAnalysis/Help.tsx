import { Typography } from 'antd';
import React from 'react';

const Help = ({ style = {} }: {style: object}): JSX.Element => {
    return (<div style={{ padding: '0 16px', lineHeight: '2.5rem', ...style }}>
        <Typography.Title level={5} >
            Guidance
        </Typography.Title>
        <div>
            <p>1. Check whether the Wait Time Ratio of a rank is greater than 0.2 (threshold).
                <p className={'left20'}>
                    (1) If yes, a communication bottleneck exists in this cluster, go to step 2 for further analysis.
                    <br/>
                    (2) If no, it can be preliminarily determined that this iteration does not have a communication bottleneck. Go to the communication matrix page to check the overall bandwidth usage.
                </p>
            </p>
            <p>
                2. Check whether the Synchronization Time Ratio Before Transit of the rank with the largest Wait Time Ratio is greater than 0.2 (threshold).
                <p className={'left20'}>
                    (1) If yes, a slow rank (the rank with the smallest Wait Time Ratio) exists. Check its forward and backward propagation time. If the forward and backward propagation time is much longer than that of other ranks, check whether the computation load is balanced and whether the processor is faulty. If the forward and backward propagation time is basically the same as that of other ranks, check the data preprocessing time.
                    <br/>
                    (2) If no, communication links are probably in low efficiency. View the communication link details of the rank with the longest Transit Time to check whether the link is faulty or the traffic volume is too small.
                </p>
            </p>
            <p>Note:
                <p className={'left20'}>
                    (1) Wait Time Ratio = Wait Time / (Wait Time + Transit Time): The larger the Wait Time Ratio, the longer the Wait Time of the rank to the total communication time, and the lower the communication efficiency.
                    <br/>
                    (2) Synchronization Time Ratio Before Transit = Synchronization Time / (Synchronization Time + Transit Time): The Synchronization Time refers to the waiting time before the first data transmission. A larger ratio of synchronization time indicates that the communication efficiency decreases due to slow ranks.
                    <br/>
                    (3) 0.2 is a threshold set in the tool to determine whether the Wait Time Ratio or Synchronization Time Ratio Before Transit is too large.<br/>
                </p>
            </p>
        </div>
        <div>
            <Typography.Title level={5} >
                <div>Advisor</div>
            </Typography.Title>
            <div className={'left20'}>
                There is a low efficiency issue in the current cluster communication, Waiting take up most of the time in communication action for rank 3 . The max wait ratio is: 0.92 .
                <br/>
                The reason is that the the ranks are not synchronized. Rank 2 has a lot longer computation time than other ranks.
            </div>
        </div>
    </div>);
};

export default Help;
