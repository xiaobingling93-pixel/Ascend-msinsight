import os
import torch
import numpy
import torch_npu
import transfer_to_npu

file_path = 'a.pt'
try:
    device = torch.device('cuda:0')
    torch.cuda.set_device(device)

    torch.save(torch.tensor(1), file_path)
    torch.load(file_path, map_location=device)
    print('Run successfully.')
finally:
    if os.path.exists(file_path):
        os.remove(file_path)
