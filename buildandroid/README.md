# Building OpenCPN for Android

These instructions describe how to build OpenCPN for Android on Ubuntu 24.04.

## Build Environment Setup

1. Required Tools:
```bash
# Install basic build tools
sudo apt update
sudo apt install -y \
  cmake \
  git \
  gettext \
  python3-pip \
  python3-venv \
  libxcb-cursor0 \
  libxcb-cursor-dev \
  libarchive-dev \
  libjpeg-dev \
  zlib1g-dev \
  libbz2-dev \
  libsndfile1-dev \
  libssl-dev \
  libgles1 \
  default-jdk

# Create a virtual environment in your home directory
python3 -m venv ~/opencpn-buildenv

# Activate the virtual environment
source ~/opencpn-buildenv/bin/activate

# Install required Python packages
pip install --upgrade pip setuptools
pip install cloudsmith-cli

# Get Java installation path
sudo update-alternatives --config java
export JAVA_HOME=/usr/lib/jvm/java-21-openjdk-amd64
```

2. Install and Configure Android Studio
   1. Download Android Studio from https://developer.android.com/studio
   2. Extract and install according to Google's instructions for Ubuntu
   3. Launch Android Studio
   4. In the welcome screen, click on "More Actions" > "SDK Manager"
   5. In SDK Manager, under "SDK Tools" tab:
      1. Check "NDK (Side by side)"
      2. Check "Android SDK Command-line Tools"
      3. Check "CMake"
      4. Click "Apply" to download and install

```bash
# Get path to Android NDK
~/Android/Sdk/cmdline-tools/latest/bin/sdkmanager --list |grep ndk

# Set environment variables
export ANDROID_SDK_ROOT=$HOME/Android/Sdk
export ANDROID_NDK_VERSION=$(ls $ANDROID_SDK_ROOT/ndk | sort -V | tail -1)
export ANDROID_NDK_ROOT=$ANDROID_SDK_ROOT/ndk/${ANDROID_NDK_VERSION}
export PATH=$ANDROID_NDK_ROOT/toolchains/llvm/prebuilt/linux-x86_64/bin:$PATH

# API 30: Android 11 (2020)
# API 31: Android 12 (2021)
# API 33: Android 13 (2022)
# API 34: Android 14 (2023)
export OCPN_ANDROID_API=33  # Minimum Android API level to target

# Create required NDK symlink
ln -sf $ANDROID_NDK_ROOT/toolchains/llvm/prebuilt/linux-x86_64/bin/llvm-ar $ANDROID_NDK_ROOT/toolchains/llvm/prebuilt/linux-x86_64/bin/aarch64-linux-android-ar

```

## Building the Core Library

1. Clone the repository:
```bash 
git clone https://github.com/OpenCPN/OpenCPN.git
cd OpenCPN
```

2. Create build directory and configure:
```bash
# For armhf build - 32bit
mkdir build-android-armhf
cd build-android-armhf
cmake \
  -DCMAKE_BUILD_TYPE=Release \
  -DOCPN_TARGET_TUPLE="Android-armhf;${OCPN_ANDROID_API};armhf" \
  -Dtool_base="$ANDROID_SDK_ROOT/ndk/${ANDROID_NDK_VERSION}/toolchains/llvm/prebuilt/linux-x86_64" \
  ..

# For arm64 build - 64bit
mkdir build-android-arm64
cd build-android-arm64
cmake \
  -DCMAKE_BUILD_TYPE=Release \
  -DOCPN_TARGET_TUPLE="Android-arm64;${OCPN_ANDROID_API};arm64" \
  -Dtool_base="$ANDROID_SDK_ROOT/ndk/${ANDROID_NDK_VERSION}/toolchains/llvm/prebuilt/linux-x86_64" \
  ..
```

3. Build the core library:
```bash
make VERBOSE=1
```

## Building the APK

After building the core library successfully:

1. The Qt libraries are already available in the downloaded support package at:
```bash
export QT_ANDROID_ROOT=/path/to/OpenCPN/cache/OCPNAndroidCoreBuildSupport/qt5

# Create APK build directory
mkdir apk_build
cd apk_build
```

3. Generate Makefile with qmake:
```bash
$QT_ANDROID_ROOT/android_armv7/bin/qmake \
  -makefile ../buildandroid/opencpn.pro \
  -o Makefile.android -r -spec android-g++ CONFIG+=debug
```

4. Build library and prepare for APK:
```bash
make -f Makefile.android
make -f Makefile.android install INSTALL_ROOT=./apk_build
```

5. Create the APK:
```bash
$QT_ANDROID_ROOT/android_armv7/bin/androiddeployqt \
  --input ./android-libopencpn.so-deployment-settings.json \
  --output ./apk_build \
  --android-platform android-19 \
  --deployment bundled
```

The final APK will be located at `./apk_build/bin/QtApp-debug.apk`

## Common Issues and Solutions

1. If build fails with Qt errors:
   - Verify Qt for Android is properly installed
   - Check that paths in opencpn.pro match your Qt installation

2. If NDK compilation fails:
   - Verify NDK version matches (26.1.10909125)
   - Ensure ANDROID_NDK_ROOT points to correct location
   
3. Missing Android SDK components:
   - Use SDK Manager to install required API levels and build tools
   - Verify ANDROID_SDK_ROOT points to correct location

## Validation

1. Install APK to device:
```bash
adb install -r ./apk_build/bin/QtApp-debug.apk
```

2. View application logs:
```bash
adb logcat DEBUGGER_TAG:I Qt:D libopencpn.so:D *:S
```