#!/bin/bash

#Small script to load the serial driver for the thing

sudo modprobe cp210x
echo 10c4 897c | sudo tee /sys/bus/usb-serial/drivers/cp210x/new_id
