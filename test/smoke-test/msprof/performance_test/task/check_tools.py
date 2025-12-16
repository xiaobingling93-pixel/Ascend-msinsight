import glob
import subprocess

from utils.command_executor import CommandExecutor

def check_analyse_result(profiler_path, framework, level):
    if framework == "pytorch":
        import torch_npu
        torch_npu.profiler.profiler.analyse(profiler_path=profiler_path)

    elif framework == "mindspore":
        import mindspore
        mindspore.profiler.profiler.analyse(profiler_path=profiler_path)

    # profiler_output_paths = glob.glob(f"{profiler_path}/*ascend_*")
    # for path in profiler_output_paths:
    #     pass

    insight_test_list = [
        "npx playwright test tests/JointTest/timelineJointTest.spec.ts",
        "npx playwright test tests/JointTest/memoryJointTest.spec.ts",
        "npx playwright test tests/JointTest/operatorJointTest.spec.ts",
        "npx playwright test tests/JointTest/summaryJointTest.spec.ts",
        "npx playwright test tests/JointTest/communicationJointTest.spec.ts"
    ]

    insight_result=""
    if level == "max":
        for insight_test in insight_test_list:
            insight_result += run_insight(insight_test)
        return insight_result
    if level == "min":
        for insight_test in insight_test_list[:3]:
            insight_result += run_insight(insight_test)
        return insight_result

def run_insight(insight_test = str):
    result = subprocess.run(
        insight_test,
        shell=True,
        cwd="/home/profiler_performance/task/Ascend-Insight/e2e",
        stdout=subprocess.PIPE,
        stderr=subprocess.PIPE,
        text=True)

    if result.returncode == 0:
        return "✅"
    else:
        return "❌"

if __name__ == "__main__":
    insight_result = check_analyse_result("/home/profiler_performance/task/output/profiler/dp_result", "pytorch")
    print(insight_result)


