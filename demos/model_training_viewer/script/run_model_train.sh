#!/bin/bash

set -e

PYTHONPATH=src/py ./.venv/bin/python src/py/model/train.py
