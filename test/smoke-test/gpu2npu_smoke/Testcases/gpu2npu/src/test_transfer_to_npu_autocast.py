import os
import torch
import numpy
import torch_npu
from torch_npu.contrib import transfer_to_npu

device = torch.device('cuda:0')
torch.cuda.set_device(device)

a = torch.randn([3, 3], device='cuda')
b = torch.randn([3, 3], device='cuda')
c = a @ b
c_before_autocast = c.dtype

with torch.autocast('cuda'):
    c = a @ b
c_after_autocast_args = c.dtype

with torch.autocast(device_type='cuda'):
    c = a @ b
c_after_autocast_kwargs = c.dtype

if c_before_autocast != c_after_autocast_args and c_after_autocast_args == c_after_autocast_kwargs:
    print('Run successfully.')
