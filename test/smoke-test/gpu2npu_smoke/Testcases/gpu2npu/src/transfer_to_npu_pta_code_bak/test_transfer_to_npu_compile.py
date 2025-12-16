import torch
import torch_npu
from torch_npu.contrib import transfer_to_npu

try:
    import torchair
except ImportError:
    IS_TORCHAIR_INSTALLED = False
else:
    IS_TORCHAIR_INSTALLED = True

backend = 'inductor'


class Model(torch.nn.Module):
    def __init__(self):
        super().__init__()

    def forward(self, x, y):
        return torch.arange(x, y)


model = Model().cuda()
if IS_TORCHAIR_INSTALLED:
    model = torch.compile(model, backend=backend, dynamic=True)
x = torch.tensor(1).cuda()
y = torch.tensor(2).cuda()
output = model(x, y)

print('Run successfully.')
