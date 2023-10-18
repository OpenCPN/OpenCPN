set -x

if [ $# -ge 1 ]; then
  cache_dir=$1
else
  cache_dir=/usr/local
fi


if [ $# -ge 2 ]; then
  arch=$2
else
  arch="arm64;x86_64"
fi

ncpu=$(sysctl -n hw.ncpu)

echo "Installing dependencies for ${arch} into ${cache_dir}"

ogg_version="1.3.5"
vorbis_version="1.3.7"
flac_version="1.4.3"
opus_version="1.4"
blake2_version="0.98.1"
zstd_version="1.5.5"
libarchive_version="3.7.2"
mpg123_version="1.32.1"
lame_version="3.100"
libsndfile_version="1.2.2"
libusb_version="1.0.26"
openssl_version="3.0.11"
wx_version="3.2.3"

macos_deployment_target="10.15"

mkdir -p "${cache_dir}"

export MACOSX_DEPLOYMENT_TARGET=${macos_deployment_target}

#TODO:
#RPATH in cmake built libs - leads to OpenCPN not being installable
#LZMA - do we need our own?
#LZ4 - do we need it?
#libsndfile - do we need the external deps (ogg vorbis etc) at all?
#If we do, vorbis for some reason builds arm64 even when we want x86_64
#Deployment targets set to older macOS versions?

if [ ! -f libogg-${ogg_version}.tar.gz ]; then
  wget https://ftp.osuosl.org/pub/xiph/releases/ogg/libogg-${ogg_version}.tar.gz
fi
tar zxf libogg-${ogg_version}.tar.gz
cd libogg-${ogg_version}
if [[ "${arch}" = *"x86_64"* ]]; then
  arch -arch x86_64 ./configure --prefix="${cache_dir}" --disable-static --disable-debug --disable-dependency-tracking
  arch -arch x86_64 make -j ${ncpu}
  if [[ "${arch}" = *"x86_64"* ]] && [[ "${arch}" = *"arm64"* ]]; then
    mv src/.libs/libogg.0.dylib libogg.0.dylib.x86_64
    make clean
  fi
fi
if [[ "${arch}" = *"arm64"* ]]; then
  arch -arch arm64 ./configure --prefix="${cache_dir}" --disable-static --disable-debug --disable-dependency-tracking
  arch -arch arm64 make -j ${ncpu}
  if [[ "${arch}" = *"x86_64"* ]] && [[ "${arch}" = *"arm64"* ]]; then
    mv src/.libs/libogg.0.dylib libogg.0.dylib.arm64
  fi
fi
if [[ "${arch}" = *"x86_64"* ]] && [[ "${arch}" = *"arm64"* ]]; then
  lipo -create libogg.0.dylib.arm64 libogg.0.dylib.x86_64 -output src/.libs/libogg.0.dylib
fi
make install
cd ..
rm -rf libogg-${ogg_version}

if [ ! -f libvorbis-${vorbis_version}.tar.xz ]; then
  wget https://downloads.xiph.org/releases/vorbis/libvorbis-${vorbis_version}.tar.xz
fi
tar zxf libvorbis-${vorbis_version}.tar.xz
cd libvorbis-${vorbis_version}
sed -i -e "s/ -force_cpusubtype_ALL//g" configure
if [[ "${arch}" = *"x86_64"* ]]; then
  arch -arch x86_64 ./configure --prefix="${cache_dir}" --disable-debug --disable-dependency-tracking --disable-static --with-ogg=${cache_dir}
  arch -arch x86_64 make -j ${ncpu}
  if [[ "${arch}" = *"x86_64"* ]] && [[ "${arch}" = *"arm64"* ]]; then
    mv lib/.libs/libvorbisenc.2.dylib libvorbisenc.2.dylib.x86_64
    mv lib/.libs/libvorbisfile.3.dylib libvorbisfile.3.dylib.x86_64
    mv lib/.libs/libvorbis.0.dylib libvorbis.0.dylib.x86_64
    make clean
  fi
fi
if [[ "${arch}" = *"arm64"* ]]; then
  arch -arch arm64 ./configure --prefix="${cache_dir}" --disable-debug --disable-dependency-tracking --disable-static --with-ogg=${cache_dir}
  arch -arch arm64 make -j ${ncpu}
  if [[ "${arch}" = *"x86_64"* ]] && [[ "${arch}" = *"arm64"* ]]; then
    mv lib/.libs/libvorbisenc.2.dylib libvorbisenc.2.dylib.arm64
    mv lib/.libs/libvorbisfile.3.dylib libvorbisfile.3.dylib.arm64
    mv lib/.libs/libvorbis.0.dylib libvorbis.0.dylib.arm64
  fi
fi
if [[ "${arch}" = *"x86_64"* ]] && [[ "${arch}" = *"arm64"* ]]; then
  lipo -create libvorbisenc.2.dylib.arm64 libvorbisenc.2.dylib.x86_64 -output lib/.libs/libvorbisenc.2.dylib
  lipo -create libvorbisfile.3.dylib.arm64 libvorbisfile.3.dylib.x86_64 -output lib/.libs/libvorbisfile.3.dylib
  lipo -create libvorbis.0.dylib.arm64 libvorbis.0.dylib.x86_64 -output lib/.libs/libvorbis.0.dylib
fi
make install
cd ..
rm -rf libvorbis-${vorbis_version}

if [ ! -f flac-${flac_version}.tar.xz ]; then
  wget https://downloads.xiph.org/releases/flac/flac-${flac_version}.tar.xz
fi
tar zxf flac-${flac_version}.tar.xz
cd flac-${flac_version}
if [[ "${arch}" = *"x86_64"* ]]; then
  arch -arch x86_64 ./configure --prefix="${cache_dir}" --disable-debug --disable-dependency-tracking --disable-static
  arch -arch x86_64 make -j ${ncpu}
  if [[ "${arch}" = *"x86_64"* ]] && [[ "${arch}" = *"arm64"* ]]; then
    mv src/libFLAC/.libs/libFLAC.12.dylib libFLAC.12.dylib.x86_64
    mv src/libFLAC++/.libs/libFLAC++.10.dylib libFLAC++.10.dylib.x86_64
    make clean
  fi
fi
if [[ "${arch}" = *"arm64"* ]]; then
  arch -arch arm64 ./configure --prefix="${cache_dir}" --disable-debug --disable-dependency-tracking --disable-static
  arch -arch arm64 make -j ${ncpu}
  if [[ "${arch}" = *"x86_64"* ]] && [[ "${arch}" = *"arm64"* ]]; then
    mv src/libFLAC/.libs/libFLAC.12.dylib libFLAC.12.dylib.arm64
    mv src/libFLAC++/.libs/libFLAC++.10.dylib libFLAC++.10.dylib.arm64
  fi
fi
if [[ "${arch}" = *"x86_64"* ]] && [[ "${arch}" = *"arm64"* ]]; then
  lipo -create libFLAC.12.dylib.arm64 libFLAC.12.dylib.x86_64 -output src/libFLAC/.libs/libFLAC.12.dylib
  lipo -create libFLAC++.10.dylib.arm64 libFLAC++.10.dylib.x86_64 -output src/libFLAC++/.libs/libFLAC++.10.dylib
fi
make install
cd ..
rm -rf flac-${flac_version}

if [ ! -f opus-${opus_version}.tar.gz ]; then
  wget https://downloads.xiph.org/releases/opus/opus-${opus_version}.tar.gz
fi
tar zxf opus-${opus_version}.tar.gz
cd opus-${opus_version}
if [[ "${arch}" = *"x86_64"* ]]; then
  arch -arch x86_64 ./configure --prefix="${cache_dir}" --disable-debug --disable-dependency-tracking --disable-static
  arch -arch x86_64 make -j ${ncpu}
  if [[ "${arch}" = *"x86_64"* ]] && [[ "${arch}" = *"arm64"* ]]; then
    mv .libs/libopus.0.dylib libopus.0.dylib.x86_64
    make clean
  fi
fi
if [[ "${arch}" = *"arm64"* ]]; then
  arch -arch arm64 ./configure --prefix="${cache_dir}" --disable-debug --disable-dependency-tracking --disable-static
  arch -arch arm64 make -j ${ncpu}
  if [[ "${arch}" = *"x86_64"* ]] && [[ "${arch}" = *"arm64"* ]]; then
    mv .libs/libopus.0.dylib libopus.0.dylib.arm64
  fi
fi
if [[ "${arch}" = *"x86_64"* ]] && [[ "${arch}" = *"arm64"* ]]; then
  lipo -create libopus.0.dylib.arm64 libopus.0.dylib.x86_64 -output .libs/libopus.0.dylib
fi
make install
cd ..
rm -rf opus-${opus_version}

#blake2
if [ ! -f libb2-${blake2_version}.tar.gz ]; then 
  wget https://github.com/BLAKE2/libb2/releases/download/v${blake2_version}/libb2-${blake2_version}.tar.gz
fi
tar xzf libb2-${blake2_version}.tar.gz
cd libb2-${blake2_version}
if [[ "${arch}" = *"x86_64"* ]]; then
  arch -arch x86_64 ./configure --prefix="${cache_dir}" --disable-static --disable-debug --disable-dependency-tracking
  arch -arch x86_64 make -j ${ncpu}
  if [[ "${arch}" = *"x86_64"* ]] && [[ "${arch}" = *"arm64"* ]]; then
    mv src/.libs/libb2.1.dylib libb2.1.dylib.x86_64
    make clean
  fi
fi
if [[ "${arch}" = *"arm64"* ]]; then
  arch -arch arm64 ./configure --prefix="${cache_dir}" --disable-static --disable-debug --disable-dependency-tracking
  arch -arch arm64 make -j ${ncpu}
  if [[ "${arch}" = *"x86_64"* ]] && [[ "${arch}" = *"arm64"* ]]; then
    mv src/.libs/libb2.1.dylib libb2.1.dylib.arm64
  fi
fi
if [[ "${arch}" = *"x86_64"* ]] && [[ "${arch}" = *"arm64"* ]]; then
  lipo -create libb2.1.dylib.arm64 libb2.1.dylib.x86_64 -output src/.libs/libb2.1.dylib
fi
make install
cd ..
rm -rf libb2-${blake2_version}

#zstd
if [ ! -f zstd-${zstd_version}.tar.gz ]; then
  wget https://github.com/facebook/zstd/releases/download/v${zstd_version}/zstd-${zstd_version}.tar.gz
fi
tar xjf zstd-${zstd_version}.tar.gz
cd zstd-${zstd_version}
if [[ "${arch}" = *"x86_64"* ]] && [[ "${arch}" = *"arm64"* ]]; then
  CFLAGS='-arch x86_64 -arch arm64' CPPFLAGS='-arch x86_64 -arch arm64' make PREFIX=${cache_dir} -j ${ncpu}
elif [[ "${arch}" = *"x86_64"* ]]; then
  CFLAGS='-arch x86_64' CPPFLAGS='-arch x86_64' make PREFIX=${cache_dir} -j ${ncpu}
elif [[ "${arch}" = *"arm64"* ]]; then
  CFLAGS='-arch arm64' CPPFLAGS='-arch arm64' make PREFIX=${cache_dir} -j ${ncpu}
fi
make PREFIX=${cache_dir} install
cd ..
rm -rf zstd-${zstd_version}

#libarchive
if [ ! -f libarchive-${libarchive_version}.tar.xz ]; then
  wget https://www.libarchive.org/downloads/libarchive-${libarchive_version}.tar.xz
fi
tar xJf libarchive-${libarchive_version}.tar.xz
cd libarchive-${libarchive_version}
#CFLAGS='-arch x86_64' CPPFLAGS='-arch x86_64' ./configure --prefix="${cache_dir}" --without-lzo2 --without-nettle --without-xml2 --without-openssl --with-expat --with-zstd="${cache_dir}"
#make -j ${ncpu}
#mv .libs/libarchive.13.dylib libarchive.13.dylib.x86-64
#make clean
#
#CFLAGS='-arch arm64' CPPFLAGS='-arch arm64' ./configure --prefix="${cache_dir}" --without-lzo2 --without-nettle --without-xml2 --without-openssl --with-expat --with-zstd="${cache_dir}"
#make -j ${ncpu}
#mv .libs/libarchive.13.dylib libarchive.13.dylib.arm64
#
#lipo -create libarchive.13.dylib.x86-64 libarchive.13.dylib.arm64 -output .libs/libarchive.13.dylib
mkdir bld
cd bld
cmake -DCMAKE_OSX_ARCHITECTURES="${arch}" -DCMAKE_INSTALL_PREFIX=${cache_dir} -DENABLE_LZ4=false -DZSTD_INCLUDE_DIR=${cache_dir}/include -DZSTD_LIBRARY=${cache_dir}/lib/libzstd.dylib -DLIBB2_INCLUDE_DIR=${cache_dir}/include -DLIBB2_LIBRARY=${cache_dir}/lib/libb2.dylib -DCMAKE_POLICY_DEFAULT_CMP0068=NEW -DCMAKE_SKIP_BUILD_RPATH=FALSE -DCMAKE_BUILD_WITH_INSTALL_RPATH=FALSE -DCMAKE_INSTALL_RPATH=${cache_dir}/lib -DCMAKE_INSTALL_RPATH_USE_LINK_PATH=TRUE ..
#-DCMAKE_MACOSX_RPATH=FALSE ..
make -j ${ncp}
make install
cd ..
cd ..
rm -rf libarchive-${libarchive_version}

#MPG123
if [ ! -f mpg123-${mpg123_version}.tar.bz2 ]; then
  wget https://downloads.sourceforge.net/project/mpg123/mpg123//mpg123-${mpg123_version}.tar.bz2
fi
tar xJf mpg123-${mpg123_version}.tar.bz2
cd mpg123-${mpg123_version}
if [[ "${arch}" = *"x86_64"* ]]; then
  CFLAGS='-arch x86_64' CPPFLAGS='-arch x86_64' ./configure --disable-debug --disable-dependency-tracking --with-default-audio=coreaudio --prefix="${cache_dir}" --disable-components --enable-libmpg123 --with-cpu=x86-64
  make -j ${ncpu}
  if [[ "${arch}" = *"x86_64"* ]] && [[ "${arch}" = *"arm64"* ]]; then
    mv src/libmpg123/.libs/libmpg123.0.dylib libmpg123.0.dylib.x86-64
    make clean
  fi
fi
if [[ "${arch}" = *"arm64"* ]]; then
  CFLAGS='-arch arm64' CPPFLAGS='-arch arm64' ./configure --disable-debug --disable-dependency-tracking --with-default-audio=coreaudio --prefix="${cache_dir}" --disable-components --enable-libmpg123 --with-cpu=aarch64
  make -j ${ncpu}
  if [[ "${arch}" = *"x86_64"* ]] && [[ "${arch}" = *"arm64"* ]]; then
    mv src/libmpg123/.libs/libmpg123.0.dylib libmpg123.0.dylib.arm64
  fi
fi
if [[ "${arch}" = *"x86_64"* ]] && [[ "${arch}" = *"arm64"* ]]; then
  lipo -create "libmpg123.0.dylib.arm64" "libmpg123.0.dylib.x86-64" -output "src/libmpg123/.libs/libmpg123.0.dylib"
fi
make install
cd ..
rm -rf mpg123-${mpg123_version}

#libmp3lame
if [ ! -f lame-${lame_version}.tar.gz ]; then
  wget https://downloads.sourceforge.net/project/lame/lame/${lame_version}/lame-${lame_version}.tar.gz
fi
tar xjf lame-${lame_version}.tar.gz
cd lame-${lame_version}
cat > lame_init_old.patch << EOF
2d1
< lame_init_old
EOF
patch include/libmp3lame.sym lame_init_old.patch
if [[ "${arch}" = *"x86_64"* ]]; then
  CFLAGS='-arch x86_64' CPPFLAGS='-arch x86_64' ./configure --build=x86_64-darwin --disable-frontend --disable-static --enable-nasm --disable-dependency-tracking --disable-debug --prefix="${cache_dir}"
  make -j ${ncpu}
  if [[ "${arch}" = *"x86_64"* ]] && [[ "${arch}" = *"arm64"* ]]; then
    mv libmp3lame/.libs/libmp3lame.0.dylib libmp3lame.0.dylib.x86-64
    make clean
  fi
fi
if [[ "${arch}" = *"arm64"* ]]; then
  CFLAGS='-arch arm64' CPPFLAGS='-arch arm64' ./configure --build=aarch64-darwin --disable-frontend --disable-static --enable-nasm --disable-dependency-tracking --disable-debug --prefix="${cache_dir}"
  make -j ${ncpu}
  if [[ "${arch}" = *"x86_64"* ]] && [[ "${arch}" = *"arm64"* ]]; then
    mv libmp3lame/.libs/libmp3lame.0.dylib libmp3lame.0.dylib.arm64
  fi
fi
if [[ "${arch}" = *"x86_64"* ]] && [[ "${arch}" = *"arm64"* ]]; then
  lipo -create libmp3lame.0.dylib.arm64 libmp3lame.0.dylib.x86-64 -output libmp3lame/.libs/libmp3lame.0.dylib
fi
make install

cd ..
rm -rf lame-${lame_version}

#libsndfile
if [ ! -f libsndfile-${libsndfile_version}.tar.xz ]; then
  wget https://github.com/libsndfile/libsndfile/releases/download/${libsndfile_version}/libsndfile-${libsndfile_version}.tar.xz
fi
tar xJf libsndfile-${libsndfile_version}.tar.xz
cd libsndfile-${libsndfile_version}
mkdir build
cd build
cmake -DCMAKE_OSX_ARCHITECTURES="${arch}" -DCMAKE_INSTALL_PREFIX=${cache_dir} -DENABLE_EXTERNAL_LIBS=false -Dmpg123_INCLUDE_DIR="${cache_dir}/include" -Dmpg123_LIBRARY="${cache_dir}/lib/libmpg123.dylib" -DMP3LAME_INCLUDE_DIR="${cache_dir}/include" -DMP3LAME_LIBRARY="${cache_dir}/lib/libmp3lame.dylib" -DBUILD_SHARED_LIBS=1 -DCMAKE_POLICY_DEFAULT_CMP0068=NEW -DCMAKE_SKIP_BUILD_RPATH=FALSE -DCMAKE_BUILD_WITH_INSTALL_RPATH=FALSE -DCMAKE_INSTALL_RPATH=${cache_dir}/lib -DCMAKE_INSTALL_RPATH_USE_LINK_PATH=TRUE ..
#-DCMAKE_MACOSX_RPATH=FALSE ..
make -j ${ncpu}
make install
cd ..
cd ..
rm -rf libsndfile-${libsndfile_version}

#libusb
if [ ! -f libusb-${libusb_version}.tar.bz2 ]; then
  wget https://github.com/libusb/libusb/releases/download/v${libusb_version}/libusb-${libusb_version}.tar.bz2
fi
tar xJf libusb-${libusb_version}.tar.bz2
cd libusb-${libusb_version}
if [[ "${arch}" = *"x86_64"* ]]; then
  CFLAGS='-arch x86_64' CPPFLAGS='-arch x86_64' ./configure --disable-static --disable-dependency-tracking --disable-debug --prefix="${cache_dir}"
  make -j ${ncpu}
  if [[ "${arch}" = *"x86_64"* ]] && [[ "${arch}" = *"arm64"* ]]; then
    mv libusb/.libs/libusb-1.0.0.dylib libusb-1.0.0.dylib.x86-64
    make clean
  fi
fi
if [[ "${arch}" = *"arm64"* ]]; then
  CFLAGS='-arch arm64' CPPFLAGS='-arch arm64' ./configure --disable-static --disable-dependency-tracking --disable-debug --prefix="${cache_dir}"
  make -j ${ncpu}
  if [[ "${arch}" = *"x86_64"* ]] && [[ "${arch}" = *"arm64"* ]]; then
    mv libusb/.libs/libusb-1.0.0.dylib libusb-1.0.0.dylib.arm64
  fi
fi
if [[ "${arch}" = *"x86_64"* ]] && [[ "${arch}" = *"arm64"* ]]; then
  lipo -create libusb-1.0.0.dylib.arm64 libusb-1.0.0.dylib.x86-64 -output libusb/.libs/libusb-1.0.0.dylib
fi
make install
cd ..
rm -rf libusb-${libusb_version}

#openssl
if [ ! -f openssl-${openssl_version}.tar.gz ]; then
  wget https://www.openssl.org/source/openssl-${openssl_version}.tar.gz
fi
tar xjf openssl-${openssl_version}.tar.gz
cd openssl-${openssl_version}
if [[ "${arch}" = *"x86_64"* ]]; then
  ./Configure --prefix="${cache_dir}" darwin64-x86_64-cc shared 
  make -j ${ncpu}
  if [[ "${arch}" = *"x86_64"* ]] && [[ "${arch}" = *"arm64"* ]]; then
    mv libcrypto.3.dylib libcrypto.3.dylib.x86-64
    mv libssl.3.dylib libssl.3.dylib.x86-64
    make clean
  fi
fi
if [[ "${arch}" = *"arm64"* ]]; then
  ./Configure --prefix=${cache_dir} enable-rc5 zlib darwin64-arm64-cc no-asm
  make -j ${ncpu}
  if [[ "${arch}" = *"x86_64"* ]] && [[ "${arch}" = *"arm64"* ]]; then
    mv libcrypto.3.dylib libcrypto.3.dylib.arm64
    mv libssl.3.dylib libssl.3.dylib.arm64
  fi
fi
if [[ "${arch}" = *"x86_64"* ]] && [[ "${arch}" = *"arm64"* ]]; then
  lipo -create libcrypto.3.dylib.arm64 libcrypto.3.dylib.x86-64 -output libcrypto.3.dylib
  lipo -create libssl.3.dylib.arm64 libssl.3.dylib.x86-64 -output libssl.3.dylib
fi
make install
cd ..
rm -rf openssl-${openssl_version}

#wxWidgets
if [ ! -f wxWidgets-${wx_version}.tar.bz2 ]; then
  wget https://github.com/wxWidgets/wxWidgets/releases/download/v${wx_version}/wxWidgets-${wx_version}.tar.bz2
fi
tar xJf wxWidgets-${wx_version}.tar.bz2
cd wxWidgets-${wx_version}

./configure \
      --with-cxx=11 \
      --with-macosx-version-min=${macos_deployment_target} \
      --enable-unicode \
      --enable-macosx_arch=$(echo ${arch} | tr ';' ',') \
      --enable-universal_binary=$(echo ${arch} | tr ';' ',') \
      --disable-sys-libs \
      --with-osx-cocoa \
      --enable-aui \
      --disable-debug \
      --with-opengl \
      --without-subdirs \
      --prefix=${cache_dir}
make -j ${ncpu}
make install
# We are maybe going to run on x86_64 and have to be using system grep, have to change the path
sudo sed -i -e "s/^EGREP.*/EGREP=\/usr\/bin\/egrep/g" $(readlink ${cache_dir}/bin/wx-config)
cd ..
rm -rf wxWidgets-${wx_version}
