import type {
  Reporter, FullConfig, Suite, TestCase, TestResult, FullResult
} from '@playwright/test/reporter';

class MyReporter implements Reporter {
  constructor() {
    console.log('Starting');
  }

  onBegin(config: FullConfig, suite: Suite) {
    console.log(`Starting all tests: ${suite.allTests().length}`);
  }

  onTestBegin(test: TestCase) {
    console.log(`Starting test ${test.parent.title} ${test.title}`);
  }

  onTestEnd(test: TestCase, result: TestResult) {
    // 适配冒烟脚本匹配结果，不要改动
    console.log(`Finished the test:  ${test.parent.title}  ${test.title}  ${result.status}`);
  }

  onEnd(result: FullResult) {
    console.log('Finished all tests.');
  }
}
export default MyReporter;