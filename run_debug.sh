#!/bin/bash

# Set the exact same environment variables as VS Code
export LD_LIBRARY_PATH="$(pwd)/build:$LD_LIBRARY_PATH"
export OPENCPN_PLUGIN_DIRS="$(pwd)/build/plugins"
export OPENCPN_SHARED_DATA="$(pwd)/data"
export WEBKIT_DISABLE_COMPOSITING_MODE="1"
export WEBKIT_DISABLE_DMABUF_RENDERER="1"
export WEBKIT_INSPECTOR_SERVER="127.0.0.1:9222"

echo "Launching Local Debug OpenCPN..."

# Run your specifically compiled version, NOT the system version
./build/opencpn
