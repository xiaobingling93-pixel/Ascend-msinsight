import torch.cuda as c

c.is_available()

c.set_device('cuda:0')

print('Run successfully.')
