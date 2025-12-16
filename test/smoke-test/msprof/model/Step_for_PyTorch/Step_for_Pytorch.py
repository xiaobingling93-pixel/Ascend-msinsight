#!/usr/bin/python3


import torch
import sys
import torch_npu

CALCULATE_DEVICE = "npu:0"
torch_npu.npu.set_device(CALCULATE_DEVICE)
#torch.npu.set_device(CALCULATE_DEVICE)

x_data = torch.Tensor([[1.0], [2.0], [3.0]]).to(CALCULATE_DEVICE)
y_data = torch.Tensor([[2.0], [4.0], [6.0]]).to(CALCULATE_DEVICE)


class LinearModel(torch.nn.Module):
    def __init__(self):
        super(LinearModel, self).__init__()
        self.linear = torch.nn.Linear(1, 1)

    def forward(self, x):
        y_pred = self.linear(x)
        return y_pred


model = LinearModel()
model = model.to(CALCULATE_DEVICE)
criterion = torch.nn.MSELoss(reduction='sum')
optimizer = torch.optim.SGD(model.parameters(), lr=0.01)

experimental_config = torch_npu.profiler._ExperimentalConfig(
    aic_metrics=torch_npu.profiler.AiCMetrics.PipeUtilization,
    profiler_level=torch_npu.profiler.ProfilerLevel.Level2,
    l2_cache=True,
    data_simplification=False
)
with torch_npu.profiler.profile(
        on_trace_ready=torch_npu.profiler.tensorboard_trace_handler("./result_dir"),
        record_shapes=True,
        with_stack=True,
        with_flops=False,
        with_modules=False,
        experimental_config=experimental_config) as prof:
    for epoch in range(3):
        # torch_npu.npu.npu_frontend_enhance.iteration_start()
        y_pred = model(x_data)
        loss = criterion(y_pred, y_data)
        print(epoch, loss.item())

        optimizer.zero_grad()
        loss.backward()
        optimizer.step()
        prof.step()
        # torch_npu.npu.npu_frontend_enhance.iteration_end()


print('w = ', model.linear.weight.item())
print('b = ', model.linear.bias.item())
x_test = torch.Tensor([[4.0]]).to(CALCULATE_DEVICE)
y_test = model(x_test)
print('y_pred = ', y_test.data)


