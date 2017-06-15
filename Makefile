SHELL := /bin/bash
BUILDDIR=build
DESTDIR?=/
PREFIX?=/usr
LIBDIR?=/lib64
LLIBDIR=$(BUILDDIR)$(PREFIX)$(LIBDIR)
BINDIR?=$(BUILDDIR)$(PREFIX)/bin
INCDIR?=$(BUILDDIR)$(PREFIX)/include
DOCDIR?=$(BUILDDIR)$(PREFIX)/share/docs/nitro
PYTHON_LAYOUT ?= unix # unix or deb

SOFILE=$(LLIBDIR)/libnitro.so
ARFILE=$(LLIBDIR)/libnitro.a
PROG_NAMES=nitro nitro_version
PROG_FILES=$(addprefix prog/, $(addsuffix .cpp, $(PROG_NAMES)))
PROGS=$(addprefix $(BINDIR)/, $(PROG_NAMES))

CPPFLAGS:=$(CPPFLAGS) -Wall -fPIC -Iinclude -I$(shell python -c "from distutils import sysconfig; print sysconfig.get_config_var('INCLUDEPY');") -Ipython/py/nitro/include
PYLIB:=$(shell python -c "from distutils import sysconfig; print sysconfig.get_config_var('BLDLIBRARY')")

USBLIB=-lusb-1.0

OBJNAMES=node device usb error types reader xmlreader userdevice writer xmlwriter scripts version
DLLHEADERS=$(addprefix include/nitro/, $(addsuffix .h, $(OBJNAMES)))
DLLSOURCES=$(addprefix src/, $(addsuffix .cpp, $(OBJNAMES)))
DLLOBJS=$(addprefix src/, $(addsuffix .o, $(OBJNAMES))) src/hr_time.o src/bihelp.o src/ihx.o src/xutils.o


ifeq ($(dist), .el5)
UDEV_FILE:=centos.rules
else
UDEV_FILE:=fedora.rules
endif



.PHONY: all test docs INCLUDES clean udev specs tgz python

all: $(SOFILE) $(ARFILE) $(PROGS) INCLUDES

test:
	make -C test run

$(SOFILE): $(LLIBDIR) $(DLLHEADERS) $(DLLOBJS)
	g++ $(CPPFLAGS) -o $(SOFILE) --shared $(DLLOBJS) -Iinclude $(USBLIB) $(LDFLAGS) -lxerces-c -ldl -lgmp -lgmpxx \
		$(PYLIB)

$(ARFILE): $(LLIBDIR) $(DLLHEADERS) $(DLLOBJS)
	ar rcs $(ARFILE) $(DLLOBJS)

INCLUDES: $(INCDIR) $(DLLHEADERS) include/nitro.h 
	$(foreach HEADER, $(DLLHEADERS), \
	  cp $(HEADER) $(INCDIR)/nitro/;  )
	cp include/nitro.h $(INCDIR)
	cp include/nitro/versionno.h $(INCDIR)/nitro/

udev: $(BUILDDIR)/etc/udev/rules.d/60-ubixum.rules

$(BUILDDIR)/etc/udev/rules.d/60-ubixum.rules: linux/*.rules
	mkdir -p $(BUILDDIR)/etc/udev/rules.d
	cp linux/$(UDEV_FILE) $@ 


$(PROGS): $(BINDIR) $(ARFILE) $(PROG_FILES) include/nitro.h
	g++ $(CPPFLAGS) -o $@ prog/`basename $@`.cpp -Iinclude -L$(LLIBDIR) $(LDFLAGS) -lnitro $(PYLIB) 

$(LLIBDIR):
	mkdir -p $(LLIBDIR)

$(INCDIR):
	mkdir -p $(INCDIR)/nitro

$(BINDIR):
	mkdir -p $(BINDIR)
$(DOCDIR):
	mkdir -p $(DOCDIR)

python: $(SOFILE)
	make -C python

docs: $(DOCDIR) $(BINDIR)/nitro_version
	mkdir -p $(BUILDDIR)/tmp
	cat docs/docs_template.conf | sed \
		-e "s/XXNITRO_VERSIONXX/`$(BINDIR)/nitro_version`/" \
		-e "s:XXBUILD_DIRXX:$(DOCDIR):" > \
		$(BUILDDIR)/tmp/docs.conf
	doxygen $(BUILDDIR)/tmp/docs.conf

spec: $(BINDIR)/nitro_version linux/nitro-drivers.spec
	cat linux/nitro-drivers.spec | sed \
		-e "s/XXVERSIONXX/`$(BINDIR)/nitro_version`/" > \
		$(BUILDDIR)/nitro-drivers.spec

TAG ?= $(shell nitro_version)

tgz:
	git archive --prefix=nitro-drivers-$(TAG)/ $(TAG) | gzip > nitro-drivers-$(TAG).tgz
    

clean:
	rm -rf src/*.o
	rm -rf build
	rm -rf python/build

install:
	mkdir -p $(DESTDIR)$(PREFIX)
	cp -r $(BUILDDIR)$(PREFIX)/* $(DESTDIR)$(PREFIX)
	pushd python ;\
	python setup.py install --root=$(DESTDIR) --prefix=/usr --install-layout=$(PYTHON_LAYOUT)
	cp -r $(BUILDDIR)/etc/* $(DESTDIR)/etc/
