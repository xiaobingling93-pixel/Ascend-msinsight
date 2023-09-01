<script setup lang="ts">
import RemoteManager from '@remoteManager';
import Modules from '@modules';
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
    border-right: var(--border-style);
    display: flex;
    border-radius: var(--border-radius);
}

article {
    flex-grow: 1;
}

@media (min-width: 1024px) {
    .wrapper {
        flex-direction: row;
    }

    aside {
        max-width: 400px;
        min-width: 200px;
        width: 15%;
        height: 100%;
        max-height: unset;
        min-height: unset;
        flex-direction: column;
    }
}
</style>
