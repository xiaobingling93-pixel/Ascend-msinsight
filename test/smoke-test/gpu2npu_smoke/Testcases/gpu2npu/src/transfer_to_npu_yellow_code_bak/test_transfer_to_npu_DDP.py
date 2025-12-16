import os
import torch
import torchvision
from torch.nn.parallel import DistributedDataParallel as NativeDDP
import torch_npu
import transfer_to_npu

os.environ['MASTER_ADDR'] = '127.0.0.1'
os.environ['MASTER_PORT'] = '29668'

local_rank = 0
device = 'cuda:%d' % local_rank

vgg = torchvision.models.vgg16(pretrained=False)
vgg = vgg.to(device)
torch.distributed.init_process_group(
                backend='nccl',
                init_method='env://',
                world_size=1,
                rank=0)
print(device)
vgg = NativeDDP(vgg, device_ids=[device])
print('Run successfully.')
