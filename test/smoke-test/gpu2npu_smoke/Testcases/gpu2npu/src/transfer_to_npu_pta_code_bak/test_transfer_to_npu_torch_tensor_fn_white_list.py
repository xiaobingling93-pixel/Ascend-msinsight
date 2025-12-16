import torch
import torch_npu
from torch_npu.contrib import transfer_to_npu

device = torch.device('cuda:0')
torch.cuda.set_device(device)

tensor = torch.ones(())
tensor.new_empty((2, 3), device=device)

tensor = torch.ones((2,))
tensor.new_full((3, 4), 3.141592, device=device)

tensor = torch.tensor((), dtype=torch.int32)
tensor.new_ones((2, 3), device=device)

tensor = torch.ones((2,))
data = [[0, 1], [2, 3]]
tensor.new_tensor(data, device=device)

tensor = torch.tensor((), dtype=torch.float32)
tensor.new_zeros((2, 3), device=device)

tensor = torch.tensor(1).to(device)

print('Run successfully.')
