import torch
import torch_npu
from torch_npu.contrib import transfer_to_npu


class NpuModel(torch.nn.Module):
    def __init__(self):
        super(NpuModel, self).__init__()

    def forward(self, x):
        a = torch.rand(1, device='cuda:0')
        x = x + a
        return x


example_input = torch.rand(1, device='cuda:0')

model = NpuModel()
output1 = model(example_input)

script_model = torch.jit.script(model)
output2 = script_model(example_input)
print('Run successfully.')
