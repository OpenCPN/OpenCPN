#
# Build the opencpn repo
#

BRANCH          ?= master
GPG_KEY         ?= leamas@opencpn.org
DESTDIR         ?= website
GPG_HOMEDIR     ?= $(shell readlink -fn gpg)
RSYNC_RSH       ?= ssh
RSYNC_HOST      ?= amazon-ec2
REPO_URL        ?= http://leamas.fedorapeople.org/opencpn/repo

all: build sign

build:
	test -d $(CURDIR)/flatpak/repo/repo || \
	    mkdir -p $(CURDIR)/flatpak/repo/repo
	flatpak-builder --repo=flatpak/repo/repo --force-clean \
	    app org.opencpn.OpenCPN.yaml
	flatpak install -y --user --reinstall \
	    $(CURDIR)/flatpak/repo/repo org.opencpn.OpenCPN

sign:
	@test -d $(GPG_HOMEDIR) || { \
	    echo "cannot find GPG_HOMEDIR (abort)"; exit 1; \
	}
	flatpak build-sign --gpg-sign=$(GPG_KEY) --gpg-homedir=$(GPG_HOMEDIR)  \
	    flatpak/repo/repo
	flatpak build-update-repo --gpg-sign=$(GPG_KEY) --prune \
	    --gpg-homedir=$(GPG_HOMEDIR) \
	    flatpak/repo/repo
	-cp ../flatpak/repo/*flat* ../flatpak/repo/index.html flatpak/repo/
	@for file in flatpak/repo/opencpn.flatpakrepo \
	   flatpak/repo/index.html \
           flatpak/repo/opencpn.flatpakref;\
        do \
	    echo "Signing $$file"; \
	    sed -i '/GPGKey/d' $$file; \
	    echo -n "GPGKey=" >> $$file	; \
	    gpg2 --homedir=$(GPG_HOMEDIR) --export $(GPG_KEY) | \
	        base64 --wrap=0 | tr -d '\n' >> $$file; \
	done

install: .phony
	test -d $(DESTDIR) || mkdir $(DESTDIR)
	gpg2 --export -a --homedir=$(GPG_HOMEDIR) $(GPG_KEY) \
	    > $(DESTDIR)/opencpn.key
	rm -rf $(DESTDIR)/repo
	cp -ar flatpak/* $(DESTDIR)

repourl:
	sed -i '/^Url/s|=.*|=$(REPO_URL)|' repo/opencpn.flatpakref

publish:
	rsync -av --rsh="$(RSYNC_RSH)" $(DESTDIR)/ \
	    $(RSYNC_HOST):/var/www/ocpn-flatpak/website

publish-beta:
	rsync -av --rsh="$(RSYNC_RSH)" $(DESTDIR)/ \
	    $(RSYNC_HOST):/var/www/ocpn-website-beta/website

.phony:
