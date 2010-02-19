

%define debug_package %{nil}

Name:		nitro-drivers
Version:    XXVERSIONXX 
Release:	1%{?dist}
Summary:	Nitro core USB driver

Group:	    Development/Libraries
License:    Ubixum, Software	
URL:	    http://ubixum.com	
Source0:    %{name}-%{version}.tgz
BuildRoot:	%{_tmppath}/%{name}-%{version}-%{release}-root-%(%{__id_u} -n)

BuildRequires: libusb1-devel xerces-c-devel doxygen python-devel gmp-devel
Requires: libusb1 xerces-c gmp
Conflicts: python-nitro < %{version} python-nitro > %{version}
ExclusiveArch: i386 x86_64

prefix:     /usr


%ifarch i386
%define libdir /lib
%endif
%ifarch x86_64
%define libdir /lib64
%endif


%description
The Nitro-core package contains the USB driver for communicating with a Nitro device 


%package devel
Summary:        Nitro core USB development Libraries
Group:          Development/Libraries
Provides:       %{name}-devel
Requires:       nitro-drivers == %{version}

%description devel
Development header file for creating software to perform data acquisition with the 
Nitro-core USB driver.


%prep
%setup -q


%build
make BUILDDIR=$RPM_BUILD_ROOT PREFIX=%{prefix} LIBDIR=%{libdir} dist=%{dist}
make docs DOCDIR=$RPM_BUILD_ROOT/%{_docdir}/nitro/
%{__cp} README CHANGELOG $RPM_BUILD_ROOT/%{_docdir}/nitro/
rm -rf $RPM_BUILD_ROOT/tmp



%install
# rm -rf $RPM_BUILD_ROOT
# make install DESTDIR=$RPM_BUILD_ROOT
# cp -r * $RPM_BUILD_ROOT/

%clean
rm -rf $RPM_BUILD_ROOT

%post
/sbin/udevcontrol reload_rules

%files
%defattr(-,root,root)
/%{prefix}/bin/*
/%{prefix}%{libdir}/*
/etc/udev/rules.d/60-ubixum.rules
%doc
%{_docdir}/nitro

# %files doc

%files devel
%defattr(-,root,root)
/%{prefix}/include/*


%changelog
