Name:           librtr
Version:        0.6.3
Release:        1%{?dist}
Summary:        Small extensible RPKI-RTR-Client C library
Group:          Development/Libraries
License:        MIT
URL:            http://rpki.realmv6.org/
Source0:        %{name}-%{version}.tar.gz
BuildRequires:  binutils gcc tar cmake libssh-devel >= 0.5.0 doxygen
Requires:       libssh >= 0.5.0

%description
RTRlib is an open-source C implementation of the  RPKI/Router Protocol
client. The library allows one to fetch and store validated prefix origin
data from a RTR-cache and performs origin verification of prefixes. It
supports different types of transport sessions (e.g., SSH, unprotected TCP)
and is easily extendable.

%package devel
Summary:        Small extensible RPKI-RTR-Client C library. Development files
Group:          Development/Libraries
Requires:       %{name} = %{version}-%{release} libssh-devel >= 0.5.0

%description devel
RTRlib is an open-source C implementation of the  RPKI/Router Protocol
client. The library allows one to fetch and store validated prefix origin
data from a RTR-cache and performs origin verification of prefixes. It
supports different types of transport sessions (e.g., SSH, unprotected TCP)
and is easily extendable.
.
This package contains development files.

%package doc
Summary:        Small extensible RPKI-RTR-Client C library. Documentation
Group:          Development/Libraries
Requires:       %{name} = %{version}-%{release}
BuildArch:      noarch

%description doc
RTRlib is an open-source C implementation of the  RPKI/Router Protocol
client. The library allows one to fetch and store validated prefix origin
data from a RTR-cache and performs origin verification of prefixes. It
supports different types of transport sessions (e.g., SSH, unprotected TCP)
and is easily extendable.
.
This package contains documentation files.

%package -n rtr-tools
Summary:        RPKI-RTR command line tools
Group:          Development/Libraries
Requires:       %{name} = %{version}-%{release}

%description -n rtr-tools
Tools for the RTRlib
Rtrclient is command line that connects to an RPKI-RTR server and prints
protocol information and information about the fetched ROAs to the console.
rpki-rov is a command line tool that connects to an RPKI-RTR server and
allows to validate given IP prefixes and origin ASes.

%prep
if [ ! -f %{SOURCE0} ]; then
  # Build Source Tarball first
  pushd `dirname %_topdir`; tar czf %{SOURCE0} . --exclude-vcs --exclude=redhat; popd
fi
cd %{_topdir}/BUILD
rm -rf %{name}-%{version}
tar xzf %{SOURCE0}
/bin/chmod -Rf a+rX,u+w,g-w,o-w .

%build
%cmake -D CMAKE_BUILD_TYPE=Release .
make %{?_smp_mflags}

%install
%make_install
strip $RPM_BUILD_ROOT/usr/lib64/librtr.so.%{version}
strip $RPM_BUILD_ROOT/usr/bin/rpki-rov
strip $RPM_BUILD_ROOT/usr/bin/rtrclient
cp %{_topdir}/BUILD/CHANGELOG %{buildroot}/%{_docdir}/rtrlib/
cp %{_topdir}/BUILD/LICENSE %{buildroot}/%{_docdir}/rtrlib/

%check
export LD_LIBRARY_PATH=.; make test

%post -p /sbin/ldconfig

%postun -p /sbin/ldconfig

%files
%{_libdir}/lib*.so.0
%attr(755,root,root) %{_libdir}/lib*.so.0.*
%doc CHANGELOG
%doc LICENSE

%files devel
%{_libdir}/lib*.so
%attr(644,root,root) %{_libdir}/pkgconfig/rtrlib.pc
%{_includedir}/rtrlib
%doc CHANGELOG
%doc LICENSE

%files doc
%{_docdir}/rtrlib

%files -n rtr-tools
%attr(755,root,root) %{_bindir}/rtrclient
%attr(755,root,root) %{_bindir}/rpki-rov
%attr(644,root,root) %{_mandir}/man1/rtrclient.1.gz
%attr(644,root,root) %{_mandir}/man1/rpki-rov.1.gz
%doc CHANGELOG
%doc LICENSE

%changelog
* Thu Dec 14 2017 Martin Winter <mwinter@opensourcerouting.org> - %{version}-%{release}
- RPM Packaging added

