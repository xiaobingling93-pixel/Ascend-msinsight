import torch_npu
import torch
from torch.utils.data import Dataset, DataLoader
from torch import nn
import torch.optim as optim
import torch.npu
import os
DEVICE_ID= 0
if os.getenv('DEVICE_ID') and str.isdigit(os.getenv('DEVICE_ID')):
    DEVICE_ID= int(os.getenv('DEVICE_ID'))
if torch.npu.current_device() != DEVICE_ID:
    torch.npu.set_device(f'npu:{DEVICE_ID}')
RANK_SIZE = int(os.getenv('RANK_SIZE'))
RANK_ID = int(os.getenv('RANK_ID'))
torch.distributed.init_process_group('hccl', rank=RANK_ID, world_size=RANK_SIZE)


class RandomDataset(Dataset):
    def __init__(self, size, length):
        self.len = length
        self.data = torch.randn(length, size)

    def __getitem__(self, index):
        return self.data[index]

    def __len__(self):
        return self.len


class Model(nn.Module):
    def __init__(self, input_size, output_size):
        super(Model, self).__init__()
        self.fc = nn.Linear(input_size, output_size)

    def forward(self, x):
        x = self.fc(x)
        return x


input_size = 5
output_size = 2
batch_size = 10
data_size = 100

rand_loader = DataLoader(dataset=RandomDataset(input_size, data_size),
                         batch_size=batch_size, shuffle=False, pin_memory = True, drop_last = True, sampler = torch.utils.data.distributed.DistributedSampler(RandomDataset(input_size, data_size)))

model = Model(input_size, output_size).npu()
model = model.npu()
if not isinstance(model, torch.nn.parallel.DistributedDataParallel):
    model = torch.nn.parallel.DistributedDataParallel(model, device_ids=[DEVICE_ID], broadcast_buffers=False)

criterion = nn.MSELoss()
optimizer = optim.SGD(model.parameters(), lr=0.01)

for epoch in range(2):
    rand_loader.sampler.set_epoch(epoch)
    for i, data in enumerate(rand_loader):
        # get the inputs
        inputs = data.npu()

        # zero the parameter gradients
        optimizer.zero_grad()

        # forward + backward + optimize
        outputs = model(inputs)
        loss = criterion(outputs, torch.randn(batch_size, output_size).npu())
        loss.backward()
        optimizer.step()

        # print statistics
        print('Epoch: %d | Loss: %.3f' % (epoch + 1, loss.item()))

print('Finished Training')
