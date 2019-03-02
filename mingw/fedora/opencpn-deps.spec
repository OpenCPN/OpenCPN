Name:		opencpn-deps
Version:	0.1
Release:	1%{?dist}
Summary:	Empty package with opencpn build dependencies

License:	MIT
URL:		https://github.com/leamas/opencpn

BuildRequires: cmake
BuildRequires: gettext
BuildRequires: make
BuildRequires: mingw-binutils-generic
BuildRequires: mingw-filesystem-base
BuildRequires: mingw32-binutils
BuildRequires: mingw32-bzip2
BuildRequires: mingw32-cairo
BuildRequires: mingw32-curl
BuildRequires: mingw32-expat
BuildRequires: mingw32-filesystem
BuildRequires: mingw32-fontconfig
BuildRequires: mingw32-freetype
BuildRequires: mingw32-gcc
BuildRequires: mingw32-gcc-c++
BuildRequires: mingw32-cpp
BuildRequires: mingw32-gettext
BuildRequires: mingw32-headers
BuildRequires: mingw32-glib2
BuildRequires: mingw32-gtk2
BuildRequires: mingw32-libarchive
BuildRequires: mingw32-libexif
BuildRequires: mingw32-libffi
BuildRequires: mingw32-libpng
BuildRequires: mingw32-libtiff
BuildRequires: mingw32-libxml2
BuildRequires: mingw32-nsiswrapper
BuildRequires: mingw32-openssl
BuildRequires: mingw32-pcre
BuildRequires: mingw32-pixman
BuildRequires: mingw32-sqlite
BuildRequires: mingw32-win-iconv
BuildRequires: mingw32-wxWidgets3 >= 3.0.2
BuildRequires: mingw32-xz-libs

%description

Empty package used to catch build dependencies for opencpn using
the mingw tools to create a Windows 32-bit executable

Use dnf builddep opencpn-deps.spec to install the dependencies.

%prep


%build


%install


%files
%doc COPYING


%changelog
* Fri Mar 01 2019 Alec Leamas <leamas.alec@gmail.com> - 0.1-1
- Initial release
