TEST_CASE_PATH="/home/result_dir/test_Msopst_Application"

rm -rf ${TEST_CASE_PATH}
cp -r /home/msprof_smoke_test/model/Msopst_Application_Case ${TEST_CASE_PATH}
cd ${TEST_CASE_PATH}
bash test_msopst.sh 2>&1 | tee test_Msopst_Application.log

python3 result_check.py
ret_val=$?
cd "/home/msprof_smoke_test/smoke_test/910_arm/testcase"
if [ $ret_val -eq 0 ]; then
    echo "test_Msopst_Application pass" >> result.txt
else
    echo "test_Msopst_Application fail" >> result.txt
fi
