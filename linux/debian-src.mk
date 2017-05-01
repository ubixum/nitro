
# this is not done
# OmniEye package has a full src build

SHELL := /bin/bash # so pushd works

DEB_DIR=deb-src
VERSION:=$(shell LD_LIBRARY_PATH=../build/usr/lib64 ../build/usr/bin/nitro_version)
BUILDDIR=$(DEB_DIR)/nitro-$(VERSION)
BUILD:=4

.PHONY: build 

build:
	mkdir $(DEB_DIR)
	pushd ../ ; \
	git archive  --prefix=nitro-$(VERSION)/ --format tar HEAD | (cd linux/$(DEB_DIR) && tar -xf -) 
	mkdir -p $(BUILDDIR)/debian
	sed -e 's/VER/$(VERSION)-$(BUILD)/' deb_ctrl.template > $(BUILDDIR)/debian/control
	sed -e 's/VER/$(VERSION)-$(BUILD)/' -e "s/DATE/$(shell date -R)/" deb-src.changelog > $(BUILDDIR)/debian/changelog
	cp deb-src.rules $(BUILDDIR)/debian/rules
	echo "9" >> $(BUILDDIR)/debian/compat
	tar -C $(DEB_DIR) -czf nitro_$(VERSION).orig.tar.gz nitro-$(VERSION) 
	cd $(BUILDDIR) && dpkg-buildpackage -us -uc

