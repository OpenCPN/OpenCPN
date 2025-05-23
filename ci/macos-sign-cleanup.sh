#!/usr/bin/env bash

set -x

echo "Cleaning up the Apple developer account information keychain"

if [ -z "${APPLE_DEVELOPER_ID}" ]; then
  echo "Apple developer account not set up, exiting..."
  exit 0
fi

KEYCHAIN_PATH="app-signing.keychain-db"

# Clean up the keychain
security delete-keychain "${KEYCHAIN_PATH}"
