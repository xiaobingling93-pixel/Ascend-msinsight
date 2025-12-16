from torch.utils.data import Dataset
from torchvision.datasets import FakeData
import numpy as np
import torchvision.transforms as transforms
from typing import Any, Callable, Optional, Tuple
import torch


class FakeDataSet(Dataset):
    def __init__(
            self,
            size: int = 1000,
            image_size: Tuple[int, int, int] = (3, 244, 244),
            num_classes: int = 10,
            transform: Optional[Callable] = None
    ) -> None:
        self.fake_dataset = FakeData(size, image_size, num_classes)
        if transform is None:
            self.transform = transforms.ToTensor()
        else:
            self.transform = transform

        self.images = [self.transform(data[0]) for data in self.fake_dataset]
        self.labels = [data[1] for data in self.fake_dataset]

    def __len__(self):
        return len(self.images)

    def __getitem__(self, item):
        image = self.images[item]
        label = torch.from_numpy(np.array(self.labels[item], dtype=np.int64))
        return image, label
