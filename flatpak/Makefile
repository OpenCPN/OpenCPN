#
# Build and install Flatpak OpenCPN
#
# NOTE: this is supposed to be run from the build dir, like
#
#   cd build; make -f ../flatpak/Makefile ...
#

all: local-build

build: local-build

build-only:
	flatpak-builder --repo=$(CURDIR)/repo --force-clean \
	    --build-only --keep-build-dirs --default-branch=devel \
	    app ../flatpak/org.opencpn.OpenCPN.yaml

submodules: .phony
	cd ../flatpak; \
	    git submodule update --init org.opencpn.OpenCPN; \
	    git submodule update --remote --rebase org.opencpn.OpenCPN; \
	cd ../flatpak/org.opencpn.OpenCPN;  \
	    git submodule update --init shared-modules; \
	    git submodule  update --remote --merge shared-modules

manifest: submodules
	cd ../flatpak; \
	    cp org.opencpn.OpenCPN/org.opencpn.OpenCPN.yaml . ; \
	    sed -i \
	        -e 's|patches/|org.opencpn.OpenCPN/patches/|g' \
	        -e 's|shared-modules/|org.opencpn.OpenCPN/shared-modules/|g' \
	        -e '/name: opencpn/,$$s/tag:.*/branch: master/' \
	        -e '/name: opencpn/,$$s/commit:.*/branch: master/' \
	             org.opencpn.OpenCPN.yaml
ci-build: manifest
	flatpak-builder --repo=$(CURDIR)/repo --force-clean \
	    --default-branch=devel \
	    app ../flatpak/org.opencpn.OpenCPN.yaml
	-flatpak uninstall --user -y org.opencpn.OpenCPN.Locale//devel
	flatpak install -y --user --reinstall \
	    $(CURDIR)/repo org.opencpn.OpenCPN//devel

local-manifest:  manifest
	cd ../flatpak; \
	    sed -i \
	        -e '/name: opencpn/,$$s/url:.*/url: ../' \
	        -e '/name: opencpn/,$$s/tag:.*/branch: HEAD/' \
	        -e '/name: opencpn/,$$s/commit:.*/branch: HEAD/' \
	             org.opencpn.OpenCPN.yaml

local-build: local-manifest
	flatpak-builder --repo=$(CURDIR)/repo --force-clean \
	    --default-branch=devel \
	    app ../flatpak/org.opencpn.OpenCPN.yaml
	-flatpak uninstall --user -y org.opencpn.OpenCPN.Locale//devel
	flatpak install -y --user --reinstall \
	    $(CURDIR)/repo org.opencpn.OpenCPN//devel

.phony:
