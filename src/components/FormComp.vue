<script setup lang="ts">
import { ref } from 'vue';
import Button from '@components/ButtonComp.vue';
import DeleteIcon from '@components/icons/bin_icon.vue';
import AddIcon from '@components/icons/cross_icon.vue';
const defaultValue = 'Please enter';
const inputs = ref([defaultValue]);

const buttonAttrs = {
    borderRadius: '50%',
    width: '2rem',
    height: '2rem'
}

function deleteInput(index: number) {
    inputs.value.splice(index, 1);
};

function addInput() {
    inputs.value.push(defaultValue);
}
</script>

<template>
    <div class="form">
        <div class="form-row">
            <div class="form-col">
                <label for="ip">IP:</label>
                <input id="ip" type="text" :placeholder="defaultValue" />
            </div>
            <div class="form-col">
                <label for="port">port:</label>
                <input id="port" type="text" :placeholder="defaultValue" />
            </div>
        </div>
        <div v-for="(input, index) in inputs" :key="index" class="form-row">
            <div class="form-col dynamic-line">
                <label for="data-path">DataPath:</label>
                <input id="data-path" type="text" :placeholder="input" />
                <Button :click="addInput" v-if="index === inputs.length - 1" :style="buttonAttrs">
                    <AddIcon/>
                </Button>
                <Button :click="deleteInput" v-else :type="'cancel'" :style="buttonAttrs">
                    <DeleteIcon/>
                </Button>
            </div>
        </div>
    </div>
</template>

<style scoped>
label {
    margin-right: 1rem;
}

.form-row {
    display: flex;
    justify-content: space-between;
    flex-direction: column;
    align-items: center;
}

.form-col {
    margin-bottom: 1rem;
    display: flex;
    justify-content: space-between;
    align-items: center;
}

.dynamic-line > input {
    flex-grow: 1;
}

.form-row button {
    margin-left: 1rem;
}

@media (min-width: 1400px) {
    .form-row {
        flex-direction: row;
        margin-bottom: 1rem;
    }
    
    .form-col {
        width: unset;
        margin-bottom: 0;
        margin-right: 1rem;
        flex-grow: 1;
    }
}

input[type='text'] {
    padding: 10px;
    border-radius: var(--border-radius);
    border: none;
    color: white;
    background-color: var(--color-background-medium);
    box-shadow: rgba(255, 255, 255, 0.1) 0px 4px 12px;
}

input[type='text']:focus {
    outline: none;
    box-shadow:
        #1890ff 1px 1px 5px 1px,
        #1890ff -1px -1px 5px 1px;
}
</style>
