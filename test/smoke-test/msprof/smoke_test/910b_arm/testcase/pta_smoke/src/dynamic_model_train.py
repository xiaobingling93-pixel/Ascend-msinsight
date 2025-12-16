import os
import torch
import multiprocessing as mp
import time

import torch_npu
import torch.nn as nn
import torch.optim as optim
import torch.distributed as dist
from torch.nn.parallel import DistributedDataParallel as DDP


class AllReduceTrainingModel(nn.Module):
    def __init__(self, input_size=5, hidden_size=10, output_size=5):
        super(AllReduceTrainingModel, self).__init__()
        self.network = nn.Sequential(
            nn.Linear(input_size, hidden_size),
            nn.ReLU(),
            nn.Linear(hidden_size, output_size)
        )

    def forward(self, x):
        return self.network(x)

    def get_training_data(self, rank, batch_size=10):
        """生成训练数据"""
        # 每个 rank 有不同的输入数据和标签
        inputs = torch.ones(batch_size, 5) * (rank + 1)
        labels = torch.ones(batch_size, 5) * (2 - rank)  # 让标签也有所不同

        return (inputs.npu() if torch.npu.is_available() else inputs,
                labels.npu() if torch.npu.is_available() else labels)


def test_allreduce_profiler_with_training():
    def worker(rank, world_size=2):
        # Initialization
        os.environ['MASTER_ADDR'] = '10.174.216.241'
        os.environ['MASTER_PORT'] = '55234'
        torch_npu.npu.set_device(rank)
        dist.init_process_group(backend='hccl', world_size=world_size, rank=rank)

        # 使用模型类
        model = AllReduceTrainingModel().npu()
        ddp_model = DDP(model, device_ids=[rank])
        optimizer = optim.SGD(ddp_model.parameters(), lr=0.001)
        criterion = nn.MSELoss()

        # 获取训练数据
        input_data, labels = model.get_training_data(rank)

        print(f"Rank {rank} before training: input mean = {input_data.mean().cpu().item()}")

        # Perform training with allreduce (through DDP)
        for j in range(20):
            time.sleep(1)
            # 训练步骤（DDP 会在 backward 和 step 时进行 allreduce）
            optimizer.zero_grad()
            outputs = ddp_model(input_data)
            loss = criterion(outputs, labels)
            loss.backward()
            optimizer.step()  # 这里会触发梯度同步的 allreduce

            print(f"Rank {rank} step {j}, loss: {loss.cpu().item()}")
        dist.destroy_process_group()

    # Start multiple processes
    processes = []
    for i in range(2):
        p = mp.Process(target=worker, args=(i,))
        p.start()
        processes.append(p)

    for p in processes:
        p.join()


if __name__ == '__main__':
    test_allreduce_profiler_with_training()
    print("model train over...")