/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2024. All rights reserved.
 */
import { createApp } from 'vue';
import { createPinia } from 'pinia';
import App from './App.vue';
import ElementPlus from 'element-plus';
import 'element-plus/dist/index.css';
import 'element-plus/theme-chalk/dark/css-vars.css';
import router from './router';
import '@/i18n';
import '@/assets/main.css';

interface CefQueryType {
    request: string;
    onSuccess: (response: string) => void;
    onFailure: (errorCode: number, errorMessage: string) => void;
};
const app = createApp(App);

app.use(createPinia());
app.use(router);
app.use(ElementPlus, { size: 'small' });

app.mount('#app');

// 禁用右键刷新以及F5、Ctrl+R刷新
document.oncontextmenu = (): boolean => false;
document.onkeydown = (event): boolean => event.key !== 'F5' && !(event.key === 'r' && event.ctrlKey);

declare global {
    interface Window {
        cefQuery: (obj: CefQueryType) => void;
    }
}
