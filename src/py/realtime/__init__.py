import torch

import logging
import socket
import struct
import typing


log = logging.getLogger(__name__)


class Float32TensorLogger:
    def __init__(self, unix_socket: str) -> None:
        self._socket = unix_socket
        self._kwargs: typing.Optional[dict[str, torch.Tensor]] = None

    def _build_payload(self, label: str, item: torch.Tensor) -> bytes:
        k_bytes_len = len(label.encode())
        v_dim_bytes_len = 4 * len(item.size())

        v_bytes = item.numpy().tobytes()
        v_bytes_len = len(v_bytes)

        total_len = 4
        for field_len in [k_bytes_len, v_dim_bytes_len, v_bytes_len]:
            total_len += 4 + field_len

        header = (
            struct.pack("<i", total_len)
            + struct.pack("<i", k_bytes_len)
            + struct.pack("<i", v_dim_bytes_len)
            + struct.pack("<i", v_bytes_len)
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
