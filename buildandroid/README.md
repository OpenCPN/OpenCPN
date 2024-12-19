# Building OpenCPN for Android

These instructions describe how to build OpenCPN for Android on Ubuntu 24.04.

## Build Environment Setup

1. Required Tools:
```bash
# Install basic build tools
sudo apt -y update
sudo apt -y install cmake git gettext python3-pip

# Create a virtual environment in your home directory
python3 -m venv ~/opencpn-buildenv

# Activate the virtual environment
source ~/opencpn-buildenv/bin/activate

# Install required Python packages
pip install --upgrade pip setuptools
pip install cloudsmith-cli

# Set environment variables 
export ANDROID_NDK_ROOT=$HOME/android-sdk/ndk/26.1.10909125
export ANDROID_SDK_ROOT=$HOME/android-sdk
export PATH=$ANDROID_NDK_ROOT/toolchains/llvm/prebuilt/linux-x86_64/bin:$PATH
```

2. Install Android SDK & NDK:
- Install Android Studio (recommended) or command line tools
- Use Android SDK Manager to install:
  - Android NDK 26.1.10909125 or higher
  - Android SDK Platform API level 16 or higher
  - Android Build Tools

## Building the Core Library

1. Clone the repository:
```bash 
git clone https://github.com/OpenCPN/OpenCPN.git
cd OpenCPN
```

2. Create build directory and configure:
```bash
mkdir build-android-armhf
cd build-android-armhf

# For armhf build
cmake \
  -DCMAKE_BUILD_TYPE=Release \
  -DOCPN_TARGET_TUPLE="Android-armhf;16;armhf" \
  -Dtool_base="$HOME/android-sdk/ndk/26.1.10909125/toolchains/llvm/prebuilt/linux-x86_64" \
  ..

# For arm64 build
cmake \
  -DCMAKE_BUILD_TYPE=Release \
  -DOCPN_TARGET_TUPLE="Android-arm64;16;arm64" \
  -Dtool_base="$HOME/android-sdk/ndk/26.1.10909125/toolchains/llvm/prebuilt/linux-x86_64" \
  ..
```

3. Build the core library:
```bash
make VERBOSE=1
```

## Building the APK

1. Install Qt for Android:
- Download Qt 5.5.0 for Android or newer
- Set up Qt Creator with Android kit

2. Configure the project:
```bash
# Set Qt environment
export QT_ANDROID_ROOT=/path/to/Qt/5.5.0

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