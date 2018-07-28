#
# Build the opencpn repo
#

BRANCH          ?= stable
GPG_KEY         ?= leamas@opencpn.org
DESTDIR         ?= website
GPG_HOMEDIR     ?= gpg
REPO_URL        ?= http://leamas.fedorapeople.org/opencpn/repo

all: build sign

build:
	flatpak-builder --force-clean app org.opencpn.OpenCPN.yaml
	flatpak build-export repo/repo app $(BRANCH)
	flatpak install -y --user --reinstall \
	    $(CURDIR)/repo/repo org.opencpn.OpenCPN
	flatpak-builder --force-clean plugin org.opencpn.OpenCPN.Plugin.Base.yaml
	flatpak build-export repo/repo plugin $(BRANCH)
	flatpak build-update-repo repo/repo
	flatpak install -y --reinstall --user \
	    $(CURDIR)/repo/repo org.opencpn.OpenCPN.Plugin.Base

sign:
	flatpak build-sign --gpg-sign=$(GPG_KEY) --gpg-homedir=$(GPG_HOMEDIR)  \
	    repo/repo
	flatpak build-update-repo --gpg-sign=$(GPG_KEY) --prune \
	    --gpg-homedir=$(GPG_HOMEDIR) \
	    repo/repo
	@for file in repo/opencpn.flatpakrepo repo/*.flatpakref; do \
	    echo "Signing $$file"; \
	    sed -i '/GPGKey/d' $$file; \
	    echo -n "GPGKey=" >> $$file	; \
	    gpg2 --homedir=$(GPG_HOMEDIR) --export $(GPG_KEY) | \
	        base64 --wrap=0 | tr -d '\n' >> $$file; \
	done

install:
	test -d $(DESTDIR) || mkdir $(DESTDIR)
	gpg2 --export -a --homedir=$(GPG_HOMEDIR) $(GPG_KEY) > $(DESTDIR)/opencpn.key
	rm -rf $(DESTDIR)/repo
	cp -ar repo/* $(DESTDIR)

repourl:
	sed -i '/^Url/s|=.*|=$(REPO_URL)|' repo/opencpn-plugin-base.flatpakref
	sed -i '/^Url/s|=.*|=$(REPO_URL)|' repo/opencpn.flatpakref
	sed -i '/^Url/s|=.*|=$(REPO_URL)|' repo/opencpn.flatpakref
