#
# Build the opencpn repo
#

BRANCH          ?= master
GPG_KEY         ?= leamas@opencpn.org
DESTDIR         ?= website
GPG_HOMEDIR     ?= $(shell readlink -fn gpg)
REPO_URL        ?= http://leamas.fedorapeople.org/opencpn/repo

all: build sign

build:
	flatpak-builder --repo=repo/repo --force-clean \
	    app org.opencpn.OpenCPN.yaml
	flatpak install -y --user --reinstall \
	    $(CURDIR)/repo/repo org.opencpn.OpenCPN

sign:
	@test -d $(GPG_HOMEDIR) || { \
	    echo "cannot find GPG_HOMEDIR (abort)"; exit 1; \
	}
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
	gpg2 --export -a --homedir=$(GPG_HOMEDIR) $(GPG_KEY) \
	    > $(DESTDIR)/opencpn.key
	rm -rf $(DESTDIR)/repo
	cp -ar repo/* $(DESTDIR)

repourl:
	sed -i '/^Url/s|=.*|=$(REPO_URL)|' repo/opencpn.flatpakref

publish:
	rsync -av $(DESTDIR)/ amazon-ec2:/var/www/ocpn-flatpak/website

publish-beta:
	rsync -av $(DESTDIR)/ amazon-ec2:/var/www/ocpn-website-beta/website
