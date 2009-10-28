# sitelib for noarch packages, sitearch for others (remove the unneeded one)
%{!?python_sitearch: %define python_sitearch %(%{__python} -c "from distutils.sysconfig import get_python_lib; print get_python_lib(1)")}

Name:		python-nitro
Version:    XXVERSIONXX 
Release:	1%{?dist}
Summary:    Python bindings for the Nitrogen Data Acquisition drivers. 

Group:		Development/Languages
License:    Ubixum, Software	
URL:	    http://ubixum.com	
Source0:    nitro-core-%{version}.tgz
BuildRoot:	%{_tmppath}/%{name}-%{version}-%{release}-root-%(%{__id_u} -n)

ExclusiveArch:  i386 x86_64 
BuildRequires:	python-devel nitro-core-devel >= %{version}
Requires:   nitro-core >= %{version}

%description
Python bindings for the Nitrogen Data Acquisition drivers.


%prep
%setup -q -n nitro-core-%{version}


%build
# Remove CFLAGS=... for noarch packages (unneeded)
cd drivers/python
CFLAGS="$RPM_OPT_FLAGS" %{__python} setup.py build


%install
rm -rf $RPM_BUILD_ROOT
cd drivers/python
%{__python} setup.py install -O1 --skip-build --root $RPM_BUILD_ROOT

 
%clean
rm -rf $RPM_BUILD_ROOT


%files
%defattr(-,root,root,-)
%doc
# For arch-specific packages: sitearch
%{python_sitearch}/*
/usr/bin/di


%changelog
