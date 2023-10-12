import torchvision.datasets as datasets

# Download the CIFAR10 training and test datasets
trainset = datasets.CIFAR10(root="data/train", train=True, download=True)
testset = datasets.CIFAR10(root="data/test", train=False, download=True)
