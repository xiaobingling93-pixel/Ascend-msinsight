import { test, expect } from '@playwright/test'

test('basic layout test', async ({ page }) => {
    await page.goto('http://localhost:3000/');
    // check basic layout by snapshot
    expect(await page.screenshot({fullPage: true})).toMatchSnapshot('mainPage.png');
});

test('new session test', async ({ page }) => {
  await page.goto('http://localhost:3000/');
  // new session page
  await page.getByRole('img', { name: 'caret-up' }).click();
  await page.waitForTimeout(1000);
  expect(await page.locator('.main>div').last().screenshot()).toMatchSnapshot('newSessionPage.png');
});

test('process select test', async ({ page }) => {
  await page.goto('http://localhost:3000/');
  // select process
  await page.locator('.processContainer > .ant-select > .ant-select-selector').click();
  await page.locator('.ant-select-item').first().click();
  await page.waitForTimeout(100);
  expect(await page.locator('.main>div').first().screenshot()).toMatchSnapshot('selectProcess.png');
});

test('time page test', async ({ page }) => {
  await page.goto('http://localhost:3000/');
  // select process
  await page.locator('.processContainer > .ant-select > .ant-select-selector').click();
  await page.locator('.ant-select-item').first().click();
  await page.waitForTimeout(200);
});

test('allocation page test', async ({ page }) => {
  await page.goto('http://localhost:3000/');
  // select process
  await page.locator('.processContainer > .ant-select > .ant-select-selector').click();
  await page.locator('.ant-select-item').first().click();
  await page.waitForTimeout(200);
});
