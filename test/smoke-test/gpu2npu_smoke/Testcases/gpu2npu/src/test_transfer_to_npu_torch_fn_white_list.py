import os
import torch
import numpy
import torch_npu
from torch_npu.contrib import transfer_to_npu

device = torch.device('cuda:0')
torch.cuda.set_device(device)

torch.logspace(start=-10, end=10, steps=5, device=device)

torch.randint(3, 5, (3,), device=device)

torch.rand(2, device=device)

torch.full_like(torch.randn(2, 3), 1.0, device=device)

torch.ones_like(torch.empty(2, 3), device=device)

torch.rand_like(torch.empty(3, 3), device=device)

torch.randperm(4, device=device)

torch.arange(5, device=device)

torch.normal(2, 3, size=(1, 4), device=device)

torch.empty_strided((2, 3), (1, 2), device=device)

torch.empty_like(torch.empty((2,3)), device=device)

torch.tril_indices(3, 3, device=device)

torch.ones(2, 3, device=device)

torch.randn(2, 3, device=device)

torch.tensor(1, device=device)

torch.triu_indices(3, 3, device=device)

a = numpy.array([1, 2, 3])
torch.as_tensor(a, device=device)

torch.zeros(2, 3, device=device)

x = torch.tensor([[1, 2], [3, 4]])
torch.randint_like(x, low=0, high=5, device=device)

torch.full((2, 3), 3.141592, device=device)

torch.eye(3, device=device)

torch.empty((2, 3), device=device)

torch.zeros_like(torch.empty(2, 3), device=device)

torch.range(1, 4, device=device)

torch.randn_like(torch.empty(3, 3), device=device)

torch.linspace(3, 10, steps=5, device=device)

print('Run successfully.')
