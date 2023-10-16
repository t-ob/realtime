import torch
import torchvision

import realtime


REALTIME_LOG = realtime.Float32TensorLogger("/tmp/example_socket")


DATA_LOADER_BATCH_SIZE = 1 << 6

# Hyperparams

HIDDEN_LAYER_SIZES = [2048, 1024, 512, 256, 128, 64]
LEARNING_RATE = 0.0001
EPOCHS = 100
DROPOUT_P = 0.15


class Network(torch.nn.Module):
    def __init__(self):
        super().__init__()

        layers = []
        fan_in = 3 * 32 * 32
        for fan_out in HIDDEN_LAYER_SIZES:
            layers.append(torch.nn.Linear(fan_in, fan_out))
            layers.append(torch.nn.Dropout(DROPOUT_P))
            # layers.append(torch.nn.BatchNorm1d(fan_out))
            layers.append(torch.nn.ReLU())
            fan_in = fan_out
        layers.append(torch.nn.Linear(HIDDEN_LAYER_SIZES[-1], 10))

        self._flatten = torch.nn.Flatten()
        self._stack = torch.nn.Sequential(*layers)

    def forward(self, x):
        x = self._flatten(x)
        logits = self._stack(x)
        return logits


def get_data_loaders(batch_size):
    transform = torchvision.transforms.Compose(
        [
            torchvision.transforms.ToTensor(),
            torchvision.transforms.Normalize((0.5, 0.5, 0.5), (0.5, 0.5, 0.5)),
        ]
    )
    train_data = torchvision.datasets.CIFAR10(
        "data/train", download=False, train=True, transform=transform
    )
    train_data_loader = torch.utils.data.DataLoader(
        train_data, batch_size=batch_size, shuffle=True, num_workers=2
    )

    test_data = torchvision.datasets.CIFAR10(
        "data/test", download=False, train=False, transform=transform
    )
    test_data_loader = torch.utils.data.DataLoader(
        test_data, batch_size=batch_size, shuffle=False, num_workers=2
    )

    return train_data_loader, test_data_loader


def test_loop(model, data_loader, loss_fn, device="cpu"):
    model.eval()
    size = len(data_loader.dataset)
    num_batches = len(data_loader)
    test_loss, correct = torch.tensor(0.0).to(device), torch.tensor(0.0).to(device)

    with torch.no_grad():
        for X, y in data_loader:
            y = y.to(device)
            pred = model(X.to(device))
            test_loss += loss_fn(pred, y)
            correct += (pred.argmax(1) == y).type(torch.float).sum().item()

    test_loss /= num_batches
    correct /= size

    REALTIME_LOG.log("test_loss", test_loss.cpu().detach())
    print(f"Test Error: \n Accuracy: {(100*correct.item()):>0.1f}%, Avg loss: {test_loss.item():>8f} \n")


def fit(model, train_data_loader, test_data_loader, loss_fn, optimiser, epochs, device="cpu"):
    size = len(train_data_loader.dataset)
    model.train()
    for epoch in range(epochs):
        for batch, (X, y) in enumerate(train_data_loader):
            pred = model(X.to(device))
            loss = loss_fn(pred, y.to(device))

            loss.backward()
            optimiser.step()
            optimiser.zero_grad()

            if batch % 100 == 0:
                current = (batch * 1) * len(X)
                REALTIME_LOG.log("train_loss", loss.cpu().detach())
                print(f"{epoch=}, {batch=}, {loss.item()=} {current=} {size=}")
        test_loop(model, test_data_loader, loss_fn, device=device)


if __name__ == "__main__":
    device = (
        "cuda"
        if torch.cuda.is_available()
        else "mps"
        if torch.backends.mps.is_available()
        else "cpu"
    )
    print(f"{device=}")

    train_data_loader, test_data_loader = get_data_loaders(DATA_LOADER_BATCH_SIZE)

    n = Network().to(device)
    loss_fn = torch.nn.CrossEntropyLoss()
    optim = torch.optim.Adam(params=n.parameters(), lr=LEARNING_RATE)

    fit(n, train_data_loader, test_data_loader, loss_fn, optim, EPOCHS, device=device)

    print("Done!")
