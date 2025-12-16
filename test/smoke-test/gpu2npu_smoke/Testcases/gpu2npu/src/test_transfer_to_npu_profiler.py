import torch.nn as nn
import torch.nn.functional as F
import torch
from utils.fake_dataset import FakeDataSet
from torch.utils.data import DataLoader
import torch_npu
from torch_npu.contrib import transfer_to_npu

import time

CALCULATE_DEVICE = "npu:0"
device = torch.device(CALCULATE_DEVICE)

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

def TestLeNetE2E():
    image_num = 6
    image_size = (3, 32, 32)
    image_classes = 10
    batch_size = 2
    lr = 0.01
    epochs = 10

    train_data = FakeDataSet(image_num, image_size, image_classes)
    train_loader = DataLoader(train_data, batch_size=batch_size)

    model = LeNet().to(device)
    criterion = nn.CrossEntropyLoss()
    optimizer = torch.optim.Adam(model.parameters(), lr=lr)
    #experimental_config = torch_npu.profiler._ExperimentalConfig()
    experimental_config = 1
    with torch.profiler.profile(
        activities=[
            torch.profiler.ProfilerActivity.CPU,
            torch.profiler.ProfilerActivity.CUDA
            ],
        schedule=torch.profiler.schedule(wait=0, warmup=0, active=2, repeat=1, skip_first=0),
        on_trace_ready=torch.profiler.tensorboard_trace_handler("./result"),

        record_shapes=False,
        profile_memory=True,
        with_stack=False,
        with_flops=False,
        with_modules=False,
        experimental_config = experimental_config) as prof:
            for epoch in range(epochs):
                train_one_epoch(epoch, epochs, train_loader, model, optimizer, criterion)
                prof.step()


if __name__ == "__main__":
    TestLeNetE2E()
    print('Run successfully.')
