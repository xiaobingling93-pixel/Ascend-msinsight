#export LD_PRELOAD=${ASCEND_TOOLKIT_HOME}/tools/mspti/lib64/libmspti.so
export LD_PRELOAD=/home/h00812463/msprof/build/prefix/mspti/libmspti.so
bash compile.sh
torchrun --nproc_per_node 4 python_monitor.py
