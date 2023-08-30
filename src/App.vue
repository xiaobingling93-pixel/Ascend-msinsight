<script setup lang="ts">
import { RouterLink, RouterView } from 'vue-router';
import RemoteManager from '@RemoteManager';
import Modules from '@Modules';
</script>

<template>
    <div class="wrapper">
        <aside>
            <RemoteManager />
        </aside>
        <article>
            <Modules />
        </article>
    </div>
</template>

<style scoped>
.wrapper {
    width: 100vw;
    height: 100vh;
    display: flex;
    flex-direction: column;
}

aside {
    min-height: 100px;
    max-height: 200px;
    height: 15%;
    border-right: 1px solid var(--color-border);
}

article {
    flex-grow: 1;
}

@media (min-width: 1024px) {
    .wrapper {
        flex-direction: row;
    }

    aside {
        max-width: calc(400px + var(--section-gap) / 8);
        min-width: calc(200px + var(--section-gap) / 8);
        width: 15%;
        height: 100%;
        max-height: unset;
        min-height: unset;
        padding-right: calc(var(--section-gap) / 8);
    }
}
</style>
