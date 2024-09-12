/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */
import React from 'react';
import { observer } from 'mobx-react';
import type { Session } from '../../entity/session';
import './Jupyter.css';
import { loading } from '../Common';
import { useTranslation } from 'react-i18next';
import { Layout } from 'ascend-layout';

const index = observer((props: { session: Session }) => {
    const { session } = props;
    const { t: tHome } = useTranslation();
    const { t } = useTranslation('jupyter');
    if (session.isIpynb && session.ipynbUrl !== '') {
        // http://localhost:端口号/lab/tree/文件相对路径/文件名.ipynb
        const urlReg = /^http:\/\/localhost:[0-9]{1,10}\/lab\/tree\/.{1,3000}\.ipynb$/;
        const isSafe = urlReg.test(session.ipynbUrl);
        if (isSafe) {
            return <div className="jupyter-div">
                <iframe id="jupyter" className="jupyter-iframe" src={session.ipynbUrl}></iframe>
                <div className="jupyter-tip">{t('Jupyter Tip')}: {session.ipynbUrl}</div>
            </div>;
        } else {
            return <Layout>
                <div style={{ marginLeft: '24px' }}>
                    <span>{tHome('Error')}: {t('Unsafe Link')} </span>
                    <span style={{ wordBreak: 'break-all' }}>{session.ipynbUrl}</span>
                </div>
            </Layout>;
        }
    } else {
        return loading;
    }
});

export default index;
