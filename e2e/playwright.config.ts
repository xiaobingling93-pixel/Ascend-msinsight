import { defineConfig, devices } from '@playwright/test';
import * as path from 'path';

/**
 * Read environment variables from file.
 * https://github.com/motdotla/dotenv
 */

/**
 * See https://playwright.dev/docs/test-configuration.
 */
const isCI = !!process.env.CI;

export default defineConfig({
    testDir: './src/tests',
    /* Maximum time one test can run for. */
    timeout: 60 * 1000,
    expect: {
        /**
         * Maximum time expect() should wait for the condition to be met.
         * For example in `await expect(locator).toHaveText();`
         */
        timeout: 5000,
    },
    /* Fail the build on CI if you accidentally left test.only in the source code. */
    forbidOnly: !!process.env.CI,
    /* Retry on CI only */
    retries: process.env.CI ? 2 : 0,
    /* Opt out of parallel tests on CI. */
    workers: process.env.CI ? 1 : 1,
    /* Reporter to use. See https://playwright.dev/docs/test-reporters */
    reporter: [
        ['html', { open: 'never' }],
        [path.resolve(__dirname, 'src/utils/myReporter.ts')],
    ],
    /* Shared settings for all the projects below. See https://playwright.dev/docs/api/class-testoptions. */
    use: {
        /* Maximum time each action such as `click()` can take. Defaults to 0 (no limit). */
        actionTimeout: 0,
        /* Base URL to use in actions like `await page.goto('/')`. */
        baseURL: 'http://localhost:5174',
        /* Collect trace when retrying the failed test. See https://playwright.dev/docs/trace-viewer */
        trace: 'on-first-retry',
        /* Only on CI systems run the tests headless */
        headless: true,
    },

    /* Configure projects for major browsers */
    projects: [
        {
            use: {
                ...devices['Desktop Chrome'],
                viewport: { width: 1920, height: 1080 },
            },
        },
    ],
    /* Run your local dev server before starting the tests */
    webServer: [
        {
            command: isCI
                ? '../server/output/linux-aarch64/bin/profiler_server --wsPort=9000 --logPath=./log'
                : `${path.resolve(__dirname, '../server/output/win_mingw64/bin/profiler_server.exe')} --wsPort=9000 --logPath=D:\\MindStudio-Insight_GUI_Test\\GUI\\Ascend-Insight`,
            reuseExistingServer: !isCI,
        },
        {
            /**
             * Use the dev server by default for faster feedback loop.
             * Use the preview server on CI for more realistic testing.
             * Playwright will re-use the local server if there is already a dev-server running.
             */
            command: 'npm run staging',
            cwd: path.resolve(__dirname, '../modules/framework'),
            url: 'http://127.0.0.1:5174',
            reuseExistingServer: !isCI,
        },
    ],
});
