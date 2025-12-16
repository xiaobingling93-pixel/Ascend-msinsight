import os
import torch
import torch.nn as nn

torch.cuda.set_device('cuda:0')

file_path = 'a.pt'

torch.save(torch.tensor(1), file_path)

a = torch.load(file_path, map_location='cuda:0')

b = torch.rand(2)
b = b.to('cuda:0')


class LinearRegressionModel(nn.Module):
    def __init__(self, input_dim, output_dim):
        super(LinearRegressionModel, self).__init__()
        self.linear = nn.Linear(input_dim, output_dim)

    def forward(self, x):
        out = self.linear(x)
        return out


input_dim = 1
output_dim = 1
model = LinearRegressionModel(input_dim, output_dim)
model = model.to('cuda:0')

if os.path.exists(file_path):
    os.remove(file_path)
print('Run successfully.')
