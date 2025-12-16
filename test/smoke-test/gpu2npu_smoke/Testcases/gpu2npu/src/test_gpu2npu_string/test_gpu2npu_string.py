import torch
import os
import torch.distributed as dist

os.environ['MASTER_ADDR'] = 'localhost'
os.environ['MASTER_PORT'] = '8888'
os.environ['RANK'] = '0'
os.environ['WORLD_SIZE'] = '1'
dist.init_process_group(backend='nccl')

backend = dist.get_backend()

device = torch.device('cuda:0')

if device.type == 'cuda' and backend == 'nccl':
    print('Run successfully.')
