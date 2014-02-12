# By default, the RPM will install to the standard REDHAWK SDR root location (/var/redhawk/sdr)
# You can override this at install time using --prefix /new/sdr/root when invoking rpm (preferred method, if you must)

#
# This file is protected by Copyright. Please refer to the COPYRIGHT file
# distributed with this source distribution.
#
# This file is part of REDHAWK.
#
# REDHAWK is free software: you can redistribute it and/or modify it
# under the terms of the GNU Lesser General Public License as published by the
# Free Software Foundation, either version 3 of the License, or (at your
# option) any later version.
#
# REDHAWK is distributed in the hope that it will be useful, but WITHOUT
# ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
# FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License
# for more details.
#
# You should have received a copy of the GNU Lesser General Public License
# along with this program.  If not, see http://www.gnu.org/licenses/.
#
%{!?_sdrroot: %define _sdrroot /var/redhawk/sdr}
%define _prefix %{_sdrroot}
Prefix: %{_prefix}

# Point install paths to locations within our target SDR root
%define _sysconfdir    %{_prefix}/etc
%define _localstatedir %{_prefix}/var
%define _mandir        %{_prefix}/man
%define _infodir       %{_prefix}/info

Name: SinkVITA49
Summary: Component %{name}
Version: 1.0.0
Release: 5%{?dist}
License: None
Group: REDHAWK/Components
Source: %{name}-%{version}.tar.gz
BuildRoot: %{_tmppath}/%{name}-root

Requires: redhawk >= 1.8.6
BuildRequires: redhawk-devel >= 1.8.6

# Interface requirements
Requires: bulkioInterfaces >= 1.8.6
BuildRequires: bulkioInterfaces >= 1.8.6

# C++ requirements
Requires: libVITA49 >= 1.0.0
BuildRequires: libVITA49-devel >= 1.0.0


%description
The SinkVITA49 REDHAWK component creates a UDP/multicast or TCP VITA49 packet stream and converts the data and SRI Keywords to IF data packets and Context packets for use within/between/outside of a REDHAWK domain application.

The Keywords for generating context packets are documented in the attached readme.txt

%prep
%setup
%build
# Implementation cpp
pushd cpp
./reconf
%ifarch x86_64
%define _bindir %{_prefix}/dom/components/SinkVITA49/cpp_x86_64
%else
%define _bindir %{_prefix}/dom/components/SinkVITA49/cpp_i686
%endif
%configure
make
popd

%install
rm -rf $RPM_BUILD_ROOT
# Implementation cpp

pushd cpp
%ifarch x86_64
%define _bindir %{_prefix}/dom/components/SinkVITA49/cpp_x86_64 
%else
%define _bindir %{_prefix}/dom/components/SinkVITA49/cpp_i686
%endif
make install DESTDIR=$RPM_BUILD_ROOT
popd

%clean
rm -rf $RPM_BUILD_ROOT

%files
%defattr(-,redhawk,redhawk)
%dir %{_prefix}/dom/components/%{name}
%{_prefix}/dom/components/%{name}/SinkVITA49.spd.xml
%{_prefix}/dom/components/%{name}/SinkVITA49.prf.xml
%{_prefix}/dom/components/%{name}/SinkVITA49.scd.xml
%ifarch x86_64
%{_prefix}/dom/components/%{name}/cpp_x86_64
%else 
%{_prefix}/dom/components/%{name}/cpp_i686
%endif
