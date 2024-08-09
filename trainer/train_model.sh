#!/bin/bash

cd ../yolov5

# ensure pyenv is initialized
export PYENV_ROOT="$HOME/.pyenv"
export PATH="$PYENV_ROOT/bin:$PATH"
eval "$(pyenv init --path)"
eval "$(pyenv init -)"

pyenv version

# python train.py -h
# python train.py \
#     --img 96 \
#     --cfg ../trainer/model.yaml \
#     --batch 32 --epochs 300 \
#     --data ../trainer/model_data.yaml \
#     --name my_training_run 

cd -
