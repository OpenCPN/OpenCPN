#!/usr/bin/env bash

set -x

echo "Importing Apple developer certificates"

if [ -z "${APPLE_DEVELOPER_ID}" ]; then
  echo "Apple developer account not set up, exiting..."
  exit 0
fi

KEYCHAIN_PATH="app-signing.keychain-db"
APP_P12="app.p12"
INST_P12="inst.p12"

# Store the certificates to the keychain
echo -n "${DEVELOPER_ID_APPLICATION_P12}" | base64 --decode -o "${APP_P12}"
echo -n "${DEVELOPER_ID_INSTALLER_P12}" | base64 --decode -o "${INST_P12}"

# Create temporary keychain
security create-keychain -p "${KEYCHAIN_PASSWORD}" "${KEYCHAIN_PATH}"
security set-keychain-settings -lut 21600 "${KEYCHAIN_PATH}"
security unlock-keychain -p "${KEYCHAIN_PASSWORD}" "${KEYCHAIN_PATH}"

# Import the Apple intermediate CA certificate into the keychain
wget https://www.apple.com/certificateauthority/DeveloperIDG2CA.cer
security import DeveloperIDG2CA.cer -k "${KEYCHAIN_PATH}"

# Import certificates to keychain
security import "${APP_P12}" -P "${DEVELOPER_ID_APPLICATION_P12_PASS}" -A -t cert -f pkcs12 -k "${KEYCHAIN_PATH}"
security import "${INST_P12}" -P "${DEVELOPER_ID_INSTALLER_P12_PASS}" -A -t cert -f pkcs12 -k "${KEYCHAIN_PATH}"
security set-key-partition-list -S apple-tool:,apple: -k "$KEYCHAIN_PASSWORD" "${KEYCHAIN_PATH}"
security list-keychain -d user -s "${KEYCHAIN_PATH}"
security default-keychain -d user -s "${KEYCHAIN_PATH}"

security find-identity -v -p codesigning

# Clean up the P12 files
rm -f "${APP_P12}" "${INST_P12}"
