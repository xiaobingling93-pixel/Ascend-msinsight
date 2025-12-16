import torch

device = torch.device('cuda:0')
tensor = torch.tensor(1)
tensor = tensor.to(device)


class MyClass:
    def __init__(self):
        self.CUDA = "CUDA"


my_instance = MyClass()

if tensor.is_cuda is True and my_instance.CUDA == my_instance.NPU:
    print('Run successfully.')
