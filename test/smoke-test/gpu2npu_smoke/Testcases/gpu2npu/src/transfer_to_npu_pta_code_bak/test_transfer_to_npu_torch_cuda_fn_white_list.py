import torch
import torch.nn as nn
import torch_npu
from torch_npu.contrib import transfer_to_npu

device = torch.device('cuda:0')
torch.cuda.set_device(device)

properties = torch.cuda.get_device_properties(device)

name = torch.cuda.get_device_name(device)

capability = torch.cuda.get_device_capability(device)

processes = torch.cuda.list_gpu_processes(device)

memory_stats = torch.cuda.memory_stats(device)

memory_summary = torch.cuda.memory_summary(device)

memory_allocated = torch.cuda.memory_allocated(device)

max_memory_allocated = torch.cuda.max_memory_allocated(device)

torch.cuda.reset_max_memory_allocated(device)

memory_reserved = torch.cuda.memory_reserved(device)

max_memory_reserved = torch.cuda.max_memory_reserved(device)

torch.cuda.reset_max_memory_cached(device)

print('Run successfully.')
