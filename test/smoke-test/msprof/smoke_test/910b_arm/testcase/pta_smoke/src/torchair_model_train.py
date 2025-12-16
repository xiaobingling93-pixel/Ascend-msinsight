import torch
import torch_npu.dynamo.torchair as torchair
from torch_npu.dynamo.torchair.configs.compiler_config import CompilerConfig

class RMSNorm(torch.nn.Module):
    def __init__(self, dim: int, eps: float = 1e-6):
        super().__init__()
        self.eps = eps
        self.weight = torch.nn.Parameter(torch.ones(dim))

    def _norm(self, x):
        return x * torch.rsqrt(x.pow(2).mean(-1, keepdim=True) + self.eps)

    def forward(self, x):
        output = self._norm(x.float()).type_as(x)
        return output * self.weight

class ToyModel(torch.nn.Module):
    def __init__(self, D_in, H, D_out):
        super(ToyModel, self).__init__()
        self.input_linear = torch.nn.Linear(D_in, H)
        self.middle_linear = torch.nn.Linear(H, H)
        self.output_linear = torch.nn.Linear(H, D_out)
        self.rms_norm = RMSNorm(D_out)

    def forward(self, x):
        h_relu = self.input_linear(x).clamp(min=0)
        for i in range(3):
            h_relu = self.middle_linear(h_relu).clamp(min=0)
        y_pred = self.output_linear(h_relu)
        y_pred = self.rms_norm(y_pred)
        return y_pred

def train():
    torch.npu.set_device(6)
    N, D_in, H, D_out = 256, 1024, 4096, 64

    input_data = torch.randn(N, D_in).npu()
    labels = torch.randn(N, D_out).npu()

    config = CompilerConfig()
    npu_backend = torchair.get_npu_backend(compiler_config=config)
    model = torch.compile(ToyModel(D_in, H, D_out).npu(), backend=npu_backend, dynamic=False)
    model(input_data)