
BUILD_DIR := ../build
BIN_DIR := $(BUILD_DIR)/usr/bin
LIB_DIR := $(BUILD_DIR)/usr/lib64
INC_DIR := $(BUILD_DIR)/usr/include
HTML_DIR := $(BUILD_DIR)/usr/share/docs/nitro/html
PY_DIR := ../python/build/lib.linux-x86_64-2.7

NITRO_VERSION := $(shell LD_LIBRARY_PATH=$(LIB_DIR) $(BIN_DIR)/nitro_version)
DEB_VER := 3
DEB_DIR := nitro_$(NITRO_VERSION)-$(DEB_VER)

CTRL := $(DEB_DIR)/DEBIAN/control



.PHONY: build_dir all

all: $(DEB_DIR).deb


build_dir:
	@mkdir -p $(DEB_DIR)/usr/lib
	@mkdir -p $(DEB_DIR)/usr/bin
	@mkdir -p $(DEB_DIR)/usr/include/nitro
	@mkdir -p $(DEB_DIR)/usr/share/doc/nitro/html
	@mkdir -p $(DEB_DIR)/usr/lib/python2.7
	@mkdir -p $(DEB_DIR)/DEBIAN

LIB_FILES := $(wildcard $(LIB_DIR)/*)
BIN_FILES := $(wildcard $(BIN_DIR)/*)
HTML_FILES := $(wildcard $(HTML_DIR)/*)
INC_FILES := $(INC_DIR)/nitro.h $(wildcard $(INC_DIR)/nitro/*)

INST_LIB_FILES := $(patsubst $(LIB_DIR)/%, $(DEB_DIR)/usr/lib/%, $(LIB_FILES))
INST_BIN_FILES := $(patsubst $(BIN_DIR)/%, $(DEB_DIR)/usr/bin/%, $(BIN_FILES))
INST_HTML_FILES := $(patsubst $(HTML_DIR)/%, $(DEB_DIR)/usr/share/doc/nitro/html/%, $(HTML_FILES))
INST_INC_FILES := $(patsubst $(INC_DIR)/%, $(DEB_DIR)/usr/include/%, $(INC_FILES))

$(DEB_DIR)/usr/lib/%: $(LIB_DIR)/% build_dir
	@cp $< $@

$(DEB_DIR)/usr/bin/%: $(BIN_DIR)/% build_dir
	@cp $< $@

$(DEB_DIR)/usr/share/doc/nitro/html/%: $(HTML_DIR)/% build_dir
	@cp $< $@

$(DEB_DIR)/usr/include/%: $(INC_DIR)/% build_dir
	@cp $< $@

$(CTRL): deb_ctrl.template build_dir
	sed -e 's/VER/$(NITRO_VERSION)-$(DEB_VER)/' $< > $@

.PHONY: install_files py

install_files: $(INST_LIB_FILES) $(INST_BIN_FILES) $(INST_HTML_FILES) $(CTRL) $(INST_INC_FILES) py

py:
	@cp -r $(PY_DIR)/* $(DEB_DIR)/usr/lib/python2.7/dist-packages/

$(DEB_DIR).deb: install_files
	dpkg-deb --build $(DEB_DIR)


clean:
	rm -rf $(DEB_DIR).deb $(DEB_DIR)
