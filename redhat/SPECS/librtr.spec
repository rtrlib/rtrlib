Name:           librtr
Version:        0.8.0
Release:        1%{?dist}
Summary:        Small extensible RPKI-RTR-Client C library
Group:          Development/Libraries
License:        MIT
URL:            http://rpki.realmv6.org/
Source0:        %{name}-%{version}.tar.gz
BuildRequires:  binutils gcc tar cmake libssh-devel >= 0.5.0 openssl-devel >= 1.0 doxygen
Requires:       libssh >= 0.5.0 openssl >= 1.0

# Fedora 33 and up changed cmake in rpm's
# See https://fedoraproject.org/wiki/Changes/CMake_to_do_out-of-source_builds
%if 0%{?fedora} >= 33 || 0%{?rhel} >= 9
    %global use_cmake_macros 1
%else
    %global use_cmake_macros 0
%endif

%description
RTRlib is an open-source C implementation of the RPKI/Router Protocol
client. The library allows one to fetch and store validated prefix origin
data from a RTR-cache and performs origin verification of prefixes. It also
allows validating and signing BGPsec AS paths. It supports different
types of transport sessions (e.g., SSH, unprotected TCP) and is easily
extendable.

%package devel
Summary:        Small extensible RPKI-RTR-Client C library. Development files
Group:          Development/Libraries
Requires:       %{name} = %{version}-%{release} libssh-devel >= 0.5.0 openssl-devel >= 1.0

%description devel
RTRlib is an open-source C implementation of the RPKI/Router Protocol
client. The library allows one to fetch and store validated prefix origin
data from a RTR-cache and performs origin verification of prefixes. It also
allows validating and signing BGPsec AS paths. It supports different
types of transport sessions (e.g., SSH, unprotected TCP) and is easily
extendable.
.
This package contains development files.

%package doc
Summary:        Small extensible RPKI-RTR-Client C library. Documentation
Group:          Development/Libraries
Requires:       %{name} = %{version}-%{release}
BuildArch:      noarch

%description doc
RTRlib is an open-source C implementation of the RPKI/Router Protocol
client. The library allows one to fetch and store validated prefix origin
data from a RTR-cache and performs origin verification of prefixes. It also
allows validating and signing BGPsec AS paths. It supports different
types of transport sessions (e.g., SSH, unprotected TCP) and is easily
extendable.
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
  mkdir -p %{_topdir}/SOURCES
  pushd `dirname %_topdir`; tar czf %{SOURCE0} --exclude-vcs --exclude=redhat . ; popd
fi
mkdir -p %{_topdir}/BUILD
cd %{_topdir}/BUILD
rm -rf %{name}-%{version}
tar xzf %{SOURCE0}
/bin/chmod -Rf a+rX,u+w,g-w,o-w .

%build
%cmake -D CMAKE_BUILD_TYPE=Release .
%if %{use_cmake_macros}
    %cmake_build %{?_smp_mflags}
%else
    make %{?_smp_mflags}
%endif

%install
%if %{use_cmake_macros}
    %cmake_install
%else
    %make_install
%endif
strip $RPM_BUILD_ROOT/usr/lib64/librtr.so.%{version}
strip $RPM_BUILD_ROOT/usr/bin/rpki-rov
strip $RPM_BUILD_ROOT/usr/bin/rtrclient
cp %{_topdir}/BUILD/CHANGELOG %{buildroot}/%{_docdir}/rtrlib/
cp %{_topdir}/BUILD/LICENSE %{buildroot}/%{_docdir}/rtrlib/

%check
%if %{use_cmake_macros}
    export LD_LIBRARY_PATH=.; %cmake_build --target test
%else
    export LD_LIBRARY_PATH=.; make test
%endif

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
# artificially modified date due to trimming all entries older than 2 years
# correct date: Mon Jan 17 2022
* Mon Jan 17 2024 Martin Winter <mwinter@opensourcerouting.org> - %%{version}-%%{release}
- Use cmake macros for builds on Fedora 33 and up and RedHat 9 and up
- Fix missing SOURCES directory during rpmbuild on newer systems

* Sun Mar 15 2020 Martin Winter <mwinter@opensourcerouting.org> - 0.7.0
- Update RPM spec changelog to fix changelog error

* Thu Dec 14 2017 Martin Winter <mwinter@opensourcerouting.org> - 0.5.0
- RPM Packaging added
