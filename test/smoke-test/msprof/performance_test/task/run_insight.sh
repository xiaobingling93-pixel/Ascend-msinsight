source ~/.bashrc
source /root/miniconda3/bin/activate performance310
cd /home/profiler_performance/task
rm -rf Ascend-Insight
git clone ssh://git@codehub-dg-y.huawei.com:2222/mindstudio/MindStudio-IDE/Ascend-Insight.git
bash Insight/start.sh &
sever_pid=$!
python /home/profiler_performance/task/Ascend-Insight/modules/build/build.py
cd /home/profiler_performance/task/Ascend-Insight/e2e
npm nstiall
npx playwright install
cd /home/profiler_performance/task/Ascend-Insight/modules/framework
npm run staging &
front_pid=$!
cd /home/profiler_performance/task
cp constants.ts Ascend-Insight/e2e/src/utils/
cp playwright.config.ts Ascend-Insight/e2e/
cd Ascend-Insight/e2e/
sleep 20
npx playwright test tests/JointTest/timelineJointTest.spec.ts
npx playwright test tests/JointTest/operatorJointTest.spec.ts
npx playwright test tests/JointTest/memoryJointTest.spec.ts
npx playwright test tests/JointTest/summaryJointTest.spec.ts
npx playwright test tests/JointTest/communicationJointTest.spec.ts

kill $sever_pid 2>/dev/null
wait $sever_pid 2>/dev/null
kill $front_pid 2>/dev/null
wait $front_pid 2>/dev/null

trap_ctrlc() {
  kill -9 ${sever_pid} && kill -9 ${front_pid}
}

trap trap_ctrlc INT
