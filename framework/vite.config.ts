/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2024. All rights reserved.
 */
import { fileURLToPath, URL } from 'node:url';
import { defineConfig } from 'vite';
import vue from '@vitejs/plugin-vue';
import vueJsx from '@vitejs/plugin-vue-jsx';
import legacy from '@vitejs/plugin-legacy';

// https://vitejs.dev/config/
export default defineConfig({
    base: '',
    plugins: [
        vue(),
        vueJsx(),
        legacy({
            targets: ['defaults', 'not IE 11'],
        }),
    ],
    resolve: {
        alias: {
            vue: fileURLToPath(new URL('./node_modules/vue/dist/vue.esm-bundler.js', import.meta.url)),
            '@': fileURLToPath(new URL('./src', import.meta.url)),
            '@modules': fileURLToPath(new URL('./src/views/ModulesView.vue', import.meta.url)),
            '@remoteManager': fileURLToPath(new URL('./src/views/RemoteManager.vue', import.meta.url)),
            '@plugins': fileURLToPath(new URL('./plugins', import.meta.url)),
            '@components': fileURLToPath(new URL('./src/components', import.meta.url)),
            '@router': fileURLToPath(new URL('./src/router/index.ts', import.meta.url)),
            '@stores': fileURLToPath(new URL('./src/stores/index.ts', import.meta.url)),
            '@centralServer': fileURLToPath(new URL('./src/centralServer', import.meta.url)),
            '@assets': fileURLToPath(new URL('./src/assets', import.meta.url)),
        },
    },
    build: {
        manifest: true,
    },
});
