/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */
module.exports = {
  root: true,
  env: {
    node: true,
  },
  extends: [
    'standard-with-typescript',
    'plugin:import/errors',
    'plugin:react/recommended',
    'plugin:react-hooks/recommended',
  ],
  plugins: [
    '@typescript-eslint',
    'react',
    'import',
  ],
  parser: '@typescript-eslint/parser',
  parserOptions: {
    ecmaVersion: 2021,
    sourceType: 'module',
    ecmaFeatures: {
      jsx: true,
    },
    project: './tsconfig.json',
  },
  settings: {
    'import/resolver': {
      node: {
        extensions: [
          '.js',
          '.jsx',
          '.ts',
          '.tsx',
        ],
      },
    },
    react: {
      version: "detect",
    }
  },
  rules: {
    'no-console': process.env.NODE_ENV === 'production' ? 'warn' : 'off',
    'no-debugger': process.env.NODE_ENV === 'production' ? 'warn' : 'off',
    indent: 'off',
    '@typescript-eslint/indent': [
      'error',
      4,
    ],
    'react/no-deprecated': 'off',
    quotes: 'off',
    '@typescript-eslint/quotes': [
      'error',
      'single',
    ],
    semi: 'off',
    '@typescript-eslint/semi': [
      'error',
      'always',
    ],
    'space-before-function-paren': 'off',
    '@typescript-eslint/space-before-function-paren': 'off',
    'no-unused-vars': 'off',
    '@typescript-eslint/no-unused-vars': ['error'],
    'comma-dangle': 'off',
    '@typescript-eslint/comma-dangle': [
      'error',
      'always-multiline',
    ],
    'max-lines': [
      'error',
      2000,
    ],
    'max-lines-per-function': [
      'warn',
      { 'max': 60, 'skipBlankLines': true, 'skipComments': true },
    ],
    'no-void': [
      'error',
      {
        allowAsStatement: true,
      },
    ],
    'no-useless-return': 'error',
    'react/prop-types': 0,
    'jsx-quotes': 1,
    'object-curly-spacing': ['error', 'always'],
    'array-bracket-spacing': ['error', 'never', { 'singleValue': false }],
    'react-hooks/exhaustive-deps': 0,
    '@typescript-eslint/member-delimiter-style': ['error', {
      "multiline": {
        "delimiter": "semi",
        "requireLast": true
      },
      "singleline": {
        "delimiter": "semi",
        "requireLast": false
      },
    }],
    '@typescript-eslint/consistent-type-definitions': ['off'],
    'n/no-callback-literal': ['off'],
    'accessor-pairs': ['off'],
    '@typescript-eslint/triple-slash-reference': ['off'],
    '@typescript-eslint/restrict-plus-operands': ['error', { allowAny: true }],
    '@typescript-eslint/prefer-function-type': ['off'],
    '@typescript-eslint/consistent-type-assertions': ['off'],
    '@typescript-eslint/restrict-template-expressions': ['off'],
    '@typescript-eslint/no-misused-promises': ['off'],
    '@typescript-eslint/promise-function-async': ['off'],
    '@typescript-eslint/no-floating-promises': ['off'],
    '@typescript-eslint/return-await': ['off'],
    '@typescript-eslint/strict-boolean-expressions': ['error', {
      allowNullableObject: true,
      allowNullableBoolean: true,
    }],
    '@typescript-eslint/no-unnecessary-type-assertion': ['off'],
    '@typescript-eslint/no-var-requires': 0,
    'import/no-unresolved':['error', { "ignore": ["@"] }],
  },
  overrides: [
    {
      files: ['**/*.ts', '**/*.tsx'],
      rules: {
        'no-undef': 'off',
      },
    },
  ],
  'ignorePatterns': ['src/dic/*'],
}
