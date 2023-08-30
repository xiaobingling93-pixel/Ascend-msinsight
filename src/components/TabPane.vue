<script setup lang="ts">
import { RouterView, RouterLink } from 'vue-router';
import { onMounted, ref } from 'vue';
import { routes } from '@router';

const routerLinkRefs = ref([]);
const navRef = ref();
onMounted(() => {
    navRef.value.querySelector('a').click();
});
</script>

<template>
    <div class="tab-pane" ref>
        <div class="tab-titles">
            <nav ref="navRef">
                <RouterLink
                    v-for="route in routes"
                    :key="`${route.name}-${route.path}`"
                    :to="route.path"
                    ref="routerLinkRefs"
                >
                    {{ route.name }}
                </RouterLink>
            </nav>
        </div>
        <div class="tab-body">
            <RouterView />
        </div>
    </div>
</template>

<style scoped>
.tab-pane {
    width: 100%;
    height: 100%;
    display: flex;
    flex-direction: column;
}

.tab-titles {
    height: 30px;
}

@media (min-height: 1024px) {
    .tab-titles {
        height: 40px;
    }
}

.tab-body {
    flex-grow: 1;
}

nav {
    width: 100%;
    height: 100%;
    font-size: 1rem;
    text-align: left;
    border-bottom: 1px solid var(--color-border);
    display: flex;
    align-items: flex-end;
}

nav a.router-link-exact-active {
    color: var(--color-text);
    background-color: var(--color-background-medium);
}

nav a {
    display: inline-block;
    border-radius: 0.5rem 0.5rem 0 0;
    padding: 0 1rem;
    border-left: 1px solid var(--color-border);
    border-right: 1px solid var(--color-border);
    border-top: 1px solid var(--color-border);
    height: 90%;
    min-width: 10rem;
    text-align: center;
    margin-right: 1px;
    text-decoration: none;
    color: var(--color-text);
}

@media (hover: hover) {
    a:hover {
        background-color: var(--color-background-medium-active);
    }
}

iframe {
    width: 100%;
    height: 100%;
    border: 0;
}
</style>
