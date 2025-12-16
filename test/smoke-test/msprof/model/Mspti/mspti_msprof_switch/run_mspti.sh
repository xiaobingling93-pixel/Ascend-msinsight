source /usr/local/Ascend/ascend-toolkit/set_env.sh
export LD_PRELOAD=${ASCEND_TOOLKIT_HOME}/lib64/libmspti.so
# export LD_PRELOAD=/home/h00812463/msprof/build/prefix/mspti/libmspti.so
python3 mnist_mspti.py --addr='127.0.0.1' --workers 10 --lr 0.8 --print-freq 1 --dist-url 'tcp://127.0.0.1:50005' --dist-backend 'hccl' --multiprocessing-distributed --world-size 1 --epochs 1 --rank 0 --device-list '4,5' --amp
