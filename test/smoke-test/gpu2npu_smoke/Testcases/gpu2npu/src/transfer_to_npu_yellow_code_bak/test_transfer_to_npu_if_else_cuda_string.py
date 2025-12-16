import torch
import torch_npu
import transfer_to_npu

device = torch.device('cuda:0')
torch.cuda.set_device(device)

a = torch.rand(2, device=device)

if device.type == 'cuda' and a.device.type == 'npu':
    print('Run successfully.')
