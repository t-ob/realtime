import torch

import logging
import socket
import struct
import zlib


log = logging.getLogger(__name__)


class Float32TensorLogger:
    ID = zlib.crc32("Float32Tensor".encode())

    def __init__(self, unix_socket: str) -> None:
        self._socket = unix_socket

    def _build_payload(self, label: str, item: torch.Tensor) -> bytes:
        label_bytes_len = len(label.encode())
        item_dim_bytes_len = 4 * len(item.size())

        v_bytes = item.numpy().tobytes()
        item_len = len(v_bytes)

        total_len = 4
        total_len += 4  # Id
        for field_len in [label_bytes_len, item_dim_bytes_len, item_len]:
            total_len += 4 + field_len

        header = (
            struct.pack("<i", total_len)
            + struct.pack("<I", self.ID)
            + struct.pack("<i", label_bytes_len)
            + struct.pack("<i", item_dim_bytes_len)
            + struct.pack("<i", item_len)
        )
        body = (
            label.encode()
            + struct.pack("<" + ("i" * len(item.size())), *item.size())
            + v_bytes
        )

        payload = header + body

        return payload

    def log(self, label: str, item: torch.Tensor):
        try:
            with socket.socket(socket.AF_UNIX, socket.SOCK_STREAM) as client_socket:
                client_socket.connect(self._socket)
                client_socket.sendall(self._build_payload(label, item))
        except Exception:
            log.exception(f"Unable to write to {self._socket}")


class StringLogger:
    ID = zlib.crc32("String".encode())

    def __init__(self, unix_socket: str) -> None:
        self._socket = unix_socket

    def _build_payload(self, label: str, item: str) -> bytes:
        k_bytes_len = len(label.encode())

        item_bytes = item.encode()
        item_bytes_len = len(item_bytes)

        total_len = 4
        total_len += 4  # Id
        for field_len in [k_bytes_len, item_bytes_len]:
            total_len += 4 + field_len

        header = (
            struct.pack("<i", total_len)
            + struct.pack("<I", self.ID)
            + struct.pack("<i", k_bytes_len)
            + struct.pack("<i", item_bytes_len)
        )
        body = (
            label.encode()
            + item_bytes
        )

        payload = header + body

        return payload

    def log(self, label: str, item: str):
        try:
            with socket.socket(socket.AF_UNIX, socket.SOCK_STREAM) as client_socket:
                client_socket.connect(self._socket)
                client_socket.sendall(self._build_payload(label, item))
        except Exception:
            log.exception(f"Unable to write to {self._socket}")