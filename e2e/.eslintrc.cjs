module.exports = {
    root: true,
    extends: [
        'eslint:recommended',
        'plugin:@typescript-eslint/recommended',
    ],
    parser: '@typescript-eslint/parser',
    parserOptions: {
        ecmaVersion: 'latest',
        sourceType: 'module',
    },
    plugins: ['@typescript-eslint'],
    rules: {
        quotes: ['error', 'single', { avoidEscape: true }],
        indent: ['error', 4],
        semi: ['error', 'always'],
        'comma-dangle': ['error', 'always-multiline'],
        'object-curly-spacing': ['error', 'always'],  // 对象字面量需要空格
        'space-before-blocks': ['error', 'always'],   // 代码块前需要空格
    },
};
