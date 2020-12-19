
Overview
-----------------------------------------------------------------------------------------------------------

General Notes
-----------------------------------------------------------------------------------------------------------

1.  Build instructions are configured for arm-v7a build only.  Some modification required for arm64-v8a build.
2.  Work directory is assumed to be "~/lib"


Build Process
-----------------------------------------------------------------------------------------------------------



DSR#### Install the Android ndk, create initial sysroot
Check https://developer.android.com/ndk/downloads and download the ndk
file. Used version: `android-ndk-r21d-linux-x86_64.zip`. Unzip the
archive to work dir (~/lib), creating `~/lib/android-ndk-r21d`.
Then, set up the initial sysroot, a copy from the ndk:

    $ cd ~/lib
    $ cp -ar android-ndk-r21d/toolchains/llvm/prebuilt/linux-x86_64/sysroot .
    $ mv sysroot android-sysroot
    $ cp -av $HOME/lib/android-ndk-r21d/platforms/android-21/arch-arm/usr/lib/* $HOME/lib/android-sysroot/usr/lib/arm-linux-androideabi

DSR#### Install Android SDK

Download the command line tools: visit https://developer.android.com/studio,
search for Command Line Tools. Download zip file and unzip it to the work
directory, creating `~/lib/cmdline-tools`.

Now, create an empty directory for the sdk:

    $ cd ~/lib
    $ mkdir android_sdk

Install platform and build tools:

    $ sdkmanager \
          --sdk_root=$HOME/lib/android_sdk --install "platforms;android-19"
    $ sdkmanager \
          --sdk_root=$HOME/lib/android_sdk --install "build-tools;21.1.2"


DSR####  Install sysroot patches
The wxWidgets configuration process for Android requires a minimally working libGL.so, and libGLU.so in the sysroot.  This step creates and installs those libraries.
Note that the libraries themselves are not actually used by the resulting Android build.
This means that this step could be skipped by adapting the wxWidgets configuration scripts.

    Get sysroot patches into ~/lib
        $ cd ~/lib
        $ git clone  git@github.com:bdbcat/sysroot_patch.git

    
    $ cd ~/lib/sysroot_patches/GL
    $ mkdir build_armv7a
    $ cmake ..
    $ make
    $ make install
    
    $ cd ~/lib/sysroot_patches/GLU
    $ mkdir build_armv7a
    $ cmake ..
    $ make
    $ make install

    
DSR#### Build Qt 5.12.10 and install into sysroot

Building from git is documented in https://wiki.qt.io/Building_Qt_5_from_Git.
After installing dependencies as documented there, do:
    
    $ git clone git://code.qt.io/qt/qt5.git
    $ cd qt5
    $ git checkout v5.12.10
    $ ./init-repository --module-subset essential,qtandroidextras  --branch
    
    Build for arm_v7a
    $ mkdir build_android_v7a
    $ cd build_android_v7a
    $ ../configure -opensource -confirm-license \
        -release \
        -xplatform android-clang \
        -android-ndk $HOME/lib/android-ndk-r21d \
        -android-sdk $HOME/Android/Sdk \
        -android-ndk-host linux-x86_64 \
        -android-ndk-platform android-19 \
        -android-arch armeabi-v7a \
        -extprefix $HOME/lib/android-sysroot/usr/Qt5 \
        -prefix /usr/Qt5 \
        -disable-rpath \
        -nomake tests -nomake examples \
        -no-warnings-are-errors \
        -skip qttranslations -skip qtserialport \
        QMAKE_CFLAGS_OPTIMIZE_SIZE=-O3
    

    $ make -j12
    $ make install
    
    
DSR#### Build wxWidgets 3.1.4 and install into sysroot
        
    $ cd ~/lib
    $ git clone  https://github.com/wxWidgets/wxWidgets.git
    $ cd wxWidgets
    $ git checkout v3.1.4
 
        
    Apply patches required for Qt/Android build model:

    $ git am wherever-this-README.md-lives/patches/wxWidgets/*
    
    

  Set environment variables    
      $ export PATH=$HOME/lib/android-ndk-r21d/toolchains/llvm/prebuilt/linux-x86_64/bin:$PATH && \
      export CXX="armv7a-linux-androideabi28-clang++ --sysroot=$HOME/lib/android-sysroot" && \
      export CXXFLAGS=" -g -O2 -pthread -fPIC -Wno-inconsistent-missing-override -Wno-potentially-evaluated-expression -fuse-ld=lld -Wno-unused-command-line-argument -DwxHAS_OPENGL_ES " && \
      export CC="armv7a-linux-androideabi28-clang --sysroot=$HOME/lib/android-sysroot" && \
      export CFLAGS=" -g -O2 -pthread -fPIC -fuse-ld=lld -Wno-unused-command-line-argument " \
      export QT5_CUSTOM_DIR=$HOME/lib/android-sysroot/usr/Qt5

 
  Configure, build and install into sysroot:

      $ ../configure --with-qt \
      --build=x86_64-unknown-linux-gnu \
      --host=arm-linux-androideabi \
      --prefix=$HOME/lib/android-sysroot/usr/local \
      --disable-shared \
      --enable-compat28 --disable-arttango --enable-image --disable-dragimage --with-libtiff=no --disable-xrc --disable-cmdline --disable-miniframe --disable-mdi --disable-stc --disable-ribbon --disable-propgrid --disable-timepick --disable-datepick --enable-intl --with-opengl --enable-selectloop --disable-detect_sm --disable-prefseditor --disable-svg --disable-fswatcher --disable-largefile --disable-precomp-headers --enable-xlocale --with-libiconv=no 
      
      
      $ make
      $ make install
 
      // wx install script does not seem to install some files/directories.
      // Do them manually...
      
        $ cp -ar ${HOME}/lib/wxWidgets/include/wx/android ${HOME}/lib/android-sysroot/usr/local/include/wx-3.1/wx && \
         cp ${HOME}/lib/wxWidgets/include/wx/unix/sound.h ${HOME}/lib/android-sysroot/usr/local/include/wx-3.1/wx/unix && \
         cp -ar ${HOME}/lib/wxWidgets/include/wx/qt/private ${HOME}/lib/android-sysroot/usr/local/include/wx-3.1/wx/qt


DSR#### Build OpenCPN native library

      Get OpenCPN source into a convenient build location
      NOTE:TODO:  PRESENTLY BUILDING FROM BRANCH "sysroot"
      
      a.  Edit buildandroid/build_android.cmake to set the correct sysroot (~/lib/android-sysroot)
      
 
      c. $ export CMAKE_PREFIX_PATH=/home/dsr/lib/android-sysroot/usr/Qt5
      
      d. $ cmake -DANDROID_ARCH=armhf \
      -DANDROID_ABI=armeabi-v7a \
      -DOCPN_USE_GARMINHOST=OFF \
      -D_wx_selected_config=androideabi-qt \
      -DCMAKE_TOOLCHAIN_FILE=../buildandroid/build_android.cmake \
      -DCMAKE_CXX_FLAGS="-fuse-ld=lld" \
      -DCMAKE_C_FLAGS="-fuse-ld=lld" \
      -DCMAKE_AR=/home/dsr/Android/Sdk/ndk-bundle/toolchains/llvm/prebuilt/linux-x86_64/bin/aarch64-linux-android-ar \
      ..


DSR#### Build OpenCPN Android APK
        TODO
        
        
        
