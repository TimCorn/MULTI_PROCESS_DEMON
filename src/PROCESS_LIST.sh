#!/bin/bash

echo "ps -aux | grep AServerDemon | grep -v grep && ps -aux | grep SClient | grep -v grep" 
ps -aux | grep AServerDemon | grep -v grep && ps -aux | grep SClient | grep -v grep





