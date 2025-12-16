import os
import time

import torch
import torch_npu

prof_data_path = os.path.join(os.getcwd(), "localhost-13_184191_20230919113614_ascend_pt")
start_time = time.time()
torch_npu.profiler.profiler.analyse(prof_data_path)
print("Duration:{}".format(time.time() - start_time))