The file "linux-atomic.o" was extracted from libgcc.a found in recent Android NDK before the global change to LLVM/Clang.

$ ar -x android-ndk-r17c/toolchains/arm-linux-androideabi-4.9/prebuilt/linux-x86_64/lib/gcc/arm-linux-androideabi/4.9.x/libgcc.a

This module must be included on armv7a build of OpenCPN NDK library to avoid undefined errors related to "__sync_fetch_and_add_4".
