import './assets/main.css';

import { createApp } from 'vue';
import { createPinia } from 'pinia';

import App from './App.vue';
import router from './router';

type CefQueryType = {request: string; onSuccess: (response: string) => void; onFailure: (errorCode: number, errorMessage: string) => void};

const app = createApp(App);

app.use(createPinia());
app.use(router);

app.mount('#app');

declare global {
    interface Window {
        cefQuery: (obj: CefQueryType) => void;
    }
};