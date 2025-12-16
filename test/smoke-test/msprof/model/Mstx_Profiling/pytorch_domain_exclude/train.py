import os
import torch.nn as nn
import torch.nn.functional as F
import torch
import torch_npu
from utils.fake_dataset import FakeDataSet
from torch.utils.data import DataLoader

import time

CALCULATE_DEVICE = "npu:0"
device = torch.device(CALCULATE_DEVICE)
torch.npu.set_device(device)
torch.npu.set_compile_mode(jit_compile=True)

class LeNet(nn.Module):
    def __init__(self):
        super(LeNet, self).__init__()
        self.conv1 = nn.Conv2d(3, 16, 5) # in_channels=3 out_channels=16 kernel=5
        self.pool1 = nn.MaxPool2d(2, 2)
        self.conv2 = nn.Conv2d(16, 32, 5)
        self.pool2 = nn.MaxPool2d(2, 2)
        self.fc1 = nn.Linear(32*5*5, 120)
        self.fc2 = nn.Linear(120, 84)
        self.fc3 = nn.Linear(84, 10)

    def forward(self, x):
        x = F.relu(self.conv1(x))    # input(3, 32, 32) output(16, 28, 28)
        x = self.pool1(x)            # output(16, 14, 14)
        x = F.relu(self.conv2(x))    # output(32, 10, 10)
        x = self.pool2(x)            # output(32, 5, 5)
        x = x.view(-1, 32*5*5)       # output(32*5*5)
        x = F.relu(self.fc1(x))      # output(120)
        x = F.relu(self.fc2(x))      # output(84)
        x = self.fc3(x)              # output(10)
        return x

def train_one_epoch(epoch, epochs, data_loader, model, optimizer, criterion):
    loss_sum = torch.tensor([0.0],dtype=torch.float32,device=device)
    acc_sum = torch.tensor([0.0],dtype=torch.float32,device=device)
    n, start = 0, time.time()
    model.train()
    for image, label in data_loader:
        image, label = image.to(device), label.to(device)
        optimizer.zero_grad()
        output = model(image)
        loss = criterion(output, label)
        loss.backward()
        optimizer.step()
    with torch.no_grad():
        label = label.long()
        loss_sum += loss.float()
        acc_sum += (torch.sum((torch.argmax(output, dim=1) == label))).float()
        n += label.shape[0]
    print("[Epoch %d/%d], Loss %.4f, Acc-Train %.3f, time: %.1f sec"%(
        epoch + 1, epochs, loss_sum/n, acc_sum/n, time.time() - start
    ))

stream = torch_npu.npu.current_stream()


def TestLeNetE2E():
    image_num = 6
    image_size = (3, 32, 32)
    image_classes = 10
    batch_size = 2
    lr = 0.01
    epochs = 10000
    msg = 'x' * 50
    train_data = FakeDataSet(image_num, image_size, image_classes)
    train_loader = DataLoader(train_data, batch_size=batch_size)

    model = LeNet().to(device)
    criterion = nn.CrossEntropyLoss()
    optimizer = torch.optim.Adam(model.parameters(), lr=lr)
    stream = torch.npu.current_stream()
    '''
    for epoch in range(epochs):
        start = time.perf_counter_ns()
        rid = torch_npu.npu.mstx.range_start(msg, stream)
        end1 = time.perf_counter_ns()
        torch_npu.npu.mstx.range_end(rid)
        end2 = time.perf_counter_ns()
        print("start: ", end1 - start)
        print("end: ", end2 - end1)
    '''
     
    experimental_config = torch_npu.profiler._ExperimentalConfig(
	aic_metrics=torch_npu.profiler.AiCMetrics.PipeUtilization,
	l2_cache=False,
	profiler_level=torch_npu.profiler.ProfilerLevel.Level_none,
	data_simplification=False,
    msprof_tx=True,
    mstx_domain_exclude=['domain2']    # 配置不采集'domain2'打点数据
    )
    with torch_npu.profiler.profile(
	activities = [torch_npu.profiler.ProfilerActivity.CPU, torch_npu.profiler.ProfilerActivity.NPU],
	schedule =torch_npu.profiler.schedule(wait=0, warmup=0, active=400, repeat=1, skip_first=10),
	on_trace_ready=torch_npu.profiler.tensorboard_trace_handler("/home/result_dir/test_Mstx_pytorch", worker_name="mstx"),
	profile_memory=True,
	with_stack=False,
	experimental_config=experimental_config) as prof:
            for epoch in range(epochs):
                torch_npu.npu.mstx.mark("mark_with_stream")

                rid = torch_npu.npu.mstx.range_start("mark_without_stream", None)
                torch_npu.npu.mstx.range_end(rid)

                rid = torch_npu.npu.mstx.range_start("mark_without_stream")
                torch_npu.npu.mstx.range_end(rid)

                rid = torch_npu.npu.mstx.range_start("mark_with_stream", stream)
                torch_npu.npu.mstx.range_end(rid)
                # 标记用户自定义domain1的mstx打点
                torch_npu.npu.mstx.mark("mark_with_domain1", domain = "domain1")
                range_id1 = torch_npu.npu.mstx.range_start("range_with_domain1", domain="domain1")
                torch_npu.npu.mstx.range_end(range_id1, domain="domain1")
                range_id1 = torch_npu.npu.mstx.range_start("range_with_domain1", domain="domain1")
                torch_npu.npu.mstx.range_end(range_id1, domain="domain1")
                range_id1 = torch_npu.npu.mstx.range_start("range_with_domain1", domain="domain1")
                torch_npu.npu.mstx.range_end(range_id1, domain="domain1")
                range_id2 = torch_npu.npu.mstx.range_start("range_with_domain2", domain="domain2")
                torch_npu.npu.mstx.range_end(range_id1, domain="domain2")
    

if __name__ == "__main__":
    TestLeNetE2E()
