import torch
from torch.utils.data import TensorDataset
import torch_npu
from torch_npu.contrib import transfer_to_npu

inps = torch.arange(10 * 5, dtype=torch.float32).view(10, 5)
tgts = torch.arange(10 * 5, dtype=torch.float32).view(10, 5)
dataset = TensorDataset(inps, tgts)

data_loader = torch.utils.data.DataLoader(
    dataset, num_workers=2, pin_memory=True)

for i, obj in enumerate(data_loader):
    print('*' * 50)
    print(obj)
    if i > 1:
        break

print('Run successfully.')
