
BUILD_DIR := ../build
BIN_DIR := $(BUILD_DIR)/usr/bin
LIB_DIR := $(BUILD_DIR)/usr/lib64
NITRO_VERSION := $(shell $(BIN_DIR)/nitro_version)
DEB_VER := 1
DEB_DIR := nitro_$(NITRO_VERSION)-$(DEB_VER)

CTRL := $(DEB_DIR)/DEBIAN/control



.PHONY: build_dir all

all: $(DEB_DIR).deb


build_dir:
	@mkdir -p $(DEB_DIR)/usr/lib
	@mkdir -p $(DEB_DIR)/usr/bin
	@mkdir -p $(DEB_DIR)/DEBIAN

LIB_FILES := $(wildcard $(LIB_DIR)/*)
BIN_FILES := $(wildcard $(BIN_DIR)/*)

INST_LIB_FILES := $(patsubst $(LIB_DIR)/%, $(DEB_DIR)/usr/lib/%, $(LIB_FILES))
INST_BIN_FILES := $(patsubst $(BIN_DIR)/%, $(DEB_DIR)/usr/bin/%, $(BIN_FILES))

$(DEB_DIR)/usr/lib/%: $(LIB_DIR)/% build_dir
	cp $< $@

$(DEB_DIR)/usr/bin/%: $(BIN_DIR)/% build_dir
	cp $< $@

$(CTRL): deb_ctrl.template build_dir
	sed -e 's/VER/$(NITRO_VERSION)-$(DEB_VER)/' $< > $@

.PHONY: install_files

install_files: $(INST_LIB_FILES) $(INST_BIN_FILES) $(CTRL)


$(DEB_DIR).deb: install_files
	dpkg-deb --build $(DEB_DIR)


clean:
	rm -rf $(DEB_DIR).deb $(DEB_DIR)
