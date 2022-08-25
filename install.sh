#!/bin/sh

mkdir -p ~/.config/clarawm
cp ./config.def.h ~/.config/clarawm/config.h
./build.sh
sudo cp ./clarawm /usr/bin

