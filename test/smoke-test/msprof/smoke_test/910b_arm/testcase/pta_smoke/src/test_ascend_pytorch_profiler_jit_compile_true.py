import torch.nn as nn
import torch.nn.functional as F
import torch
import torch_npu
from utils.fake_dataset import FakeDataSet
from torch.utils.data import DataLoader
from check_tools.file_check import FileChecker

import sys
import time
import glob
import tempfile

CALCULATE_DEVICE = "npu:0"
device = torch.device(CALCULATE_DEVICE)
torch.npu.set_device(device)
torch.npu.set_compile_mode(jit_compile=True)


class LeNet(nn.Module):
    def __init__(self):
        super(LeNet, self).__init__()
        # in_channels=3 out_channels=16 kernel=5
        self.conv1 = nn.Conv2d(3, 16, 5)
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
    loss_sum = torch.tensor([0.0], dtype=torch.float32, device=device)
    acc_sum = torch.tensor([0.0], dtype=torch.float32, device=device)
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
    print("[Epoch %d/%d], Loss %.4f, Acc-Train %.3f, time: %.1f sec" % (
        epoch + 1, epochs, loss_sum/n, acc_sum/n, time.time() - start
    ))


def test_profiler_with_jit_compile_true():
    image_num = 1 * 64
    image_size = (3, 32, 32)
    image_classes = 10
    batch_size = 64
    lr = 0.01
    epochs = 5

    train_data = FakeDataSet(image_num, image_size, image_classes)
    train_loader = DataLoader(train_data, batch_size=batch_size)

    model = LeNet().to(device)
    criterion = nn.CrossEntropyLoss()
    optimizer = torch.optim.Adam(model.parameters(), lr=lr)
    with tempfile.TemporaryDirectory() as prof_dir:
        with torch_npu.profiler.profile(
                activities=[torch_npu.profiler.ProfilerActivity.CPU,
                            torch_npu.profiler.ProfilerActivity.NPU],
                on_trace_ready=torch_npu.profiler.tensorboard_trace_handler(prof_dir),
                schedule=torch_npu.profiler.schedule(
                    wait=0, warmup=0, active=2, repeat=1, skip_first=1),
        ) as prof:
            for epoch in range(epochs):
                train_one_epoch(epoch, epochs, train_loader,
                                model, optimizer, criterion)
                prof.step()
            _check_files(prof_dir)
def _check_files(prof_dir):
    # 1. trace_view.json
    trace_view_path = glob.glob(f"{prof_dir}/*_ascend_pt/"
                                f"ASCEND_PROFILER_OUTPUT/trace_view.json")
    assert trace_view_path, f"No trace_view.json found in {prof_dir}"
    FileChecker.check_timeline_values(
        trace_view_path[0],
        "cat",
        [
            "async_npu",
            "async_task_queue",
            "fwdbwd",
            "HostToDevice",
        ],
        fuzzy_match=True
    )
    # 2. kernel_details.csv
    kernel_details_path = glob.glob(f"{prof_dir}/*_ascend_pt/"
                                    f"ASCEND_PROFILER_OUTPUT/kernel_details.csv")
    assert kernel_details_path, f"No kernel_details.csv found in {prof_dir}"
    FileChecker.check_csv_items(kernel_details_path[0], {"Name": ["Conv2D*", "*TransData*"]})
    # 3. step_trace_time.csv
    step_trace_time_path = glob.glob(f"{prof_dir}/*_ascend_pt/"
                                     f"ASCEND_PROFILER_OUTPUT/step_trace_time.csv")
    assert step_trace_time_path, f"No step_trace_time.csv found in {prof_dir}"
    FileChecker.check_csv_headers(step_trace_time_path[0], ["Step"])
    # 4. operator_details.csv
    operator_details_path = glob.glob(f"{prof_dir}/*_ascend_pt/"
                                      f"ASCEND_PROFILER_OUTPUT/operator_details.csv")
    assert operator_details_path, f"No operator_details.csv found in {prof_dir}"
    FileChecker.check_csv_items(operator_details_path[0], {"Name": ["aten*"]}, fuzzy_match=True)
    # 5. Check profiler.log
    profiler_log_paths = glob.glob(f"{prof_dir}/*_ascend_pt/"
                                   f"logs/*.log")
    assert profiler_log_paths, f"No profiler.log found in {prof_dir}"
    for profiler_log_path in profiler_log_paths:
        FileChecker.check_file_for_keyword(profiler_log_path, "error")

if __name__ == "__main__":
    test_profiler_with_jit_compile_true()
