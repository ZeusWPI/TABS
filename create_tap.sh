#!/usr/bin/env bash

sudo ip tuntap add dev tap0 mode tap
sudo ip link set up dev tap0
sudo chown $USER tap0
