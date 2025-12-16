import torch
import torch.nn as nn
import torch_npu
from torch_npu.contrib import transfer_to_npu

device = torch.device('cuda:0')
torch.cuda.set_device(device)


class SimpleNet(nn.Module):
    def __init__(self):
        super(SimpleNet, self).__init__()
        self.fc1 = nn.Linear(10, 5)
        self.fc2 = nn.Linear(5, 1)

    def forward(self, x):
        x = torch.relu(self.fc1(x))
        x = self.fc2(x)
        return x


model = SimpleNet()

model1 = model.to(device)
model2 = model.to_empty(device=device)

print('Run successfully.')
