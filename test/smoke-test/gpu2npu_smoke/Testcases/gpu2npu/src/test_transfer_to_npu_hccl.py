import torch
import os
import torch.distributed as dist
import torch_npu
from torch_npu.contrib import transfer_to_npu

os.environ['MASTER_ADDR'] = 'localhost'
os.environ['MASTER_PORT'] = '8888'
os.environ['RANK'] = '0'
os.environ['WORLD_SIZE'] = '1'
dist.init_process_group(backend='nccl')

backend = dist.get_backend()

if backend == 'hccl' and torch.distributed.is_nccl_available():
    print('Run successfully.')
