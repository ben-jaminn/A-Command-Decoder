#!/bin/bash

# --- UserLAnd Environment Setup Script ---
# Optimized for Redmi 14C

echo "Updating package lists..."
sudo apt update && sudo apt upgrade -y

echo "Installing essential build tools..."
sudo apt install -y build-essential git curl wget nano

echo "Cleaning up unnecessary cache..."
sudo apt autoremove -y && sudo apt clean

echo "Environment is ready!"
