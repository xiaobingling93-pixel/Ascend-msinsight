import './assets/main.css';

import { createApp } from 'vue';
import { createPinia } from 'pinia';
import App from './App.vue';
import ElementPlus from 'element-plus';
import 'element-plus/dist/index.css';
import 'element-plus/theme-chalk/dark/css-vars.css';
import '@assets/reset.css'
import router from './router';

type CefQueryType = {
    request: string;
    onSuccess: (response: string) => void;
    onFailure: (errorCode: number, errorMessage: string) => void;
};
const app = createApp(App);

app.use(createPinia());
app.use(router);
app.use(ElementPlus);

app.mount('#app');

// 禁用右键刷新以及F5、Ctrl+R刷新
document.oncontextmenu = () => false;
document.onkeydown = (event) => event.key !== 'F5' && !(event.key === 'r' && event.ctrlKey);

declare global {
    interface Window {
        cefQuery: (obj: CefQueryType) => void;
        request: Function;
    }
}
