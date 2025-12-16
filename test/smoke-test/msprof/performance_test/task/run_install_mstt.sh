rm -rf mstt
git clone https://gitee.com/ascend/mstt.git
cd mstt/profiler/msprof_analyze
echo "msprof-analyze code download completed."
pip uninstall -y msprof-analyze
python setup.py bdist_wheel
pip install dist/*.whl --force-reinstall
echo "msprof-analyze package installation completed."
