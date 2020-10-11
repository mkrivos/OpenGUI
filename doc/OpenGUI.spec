%define name	OpenGUI
%define ver 5.5.7
%define rel 6
%define prefix /usr/local

Name: %name
Version: %ver
Release: %rel
# Distribution:
Vendor: Marian Krivos <nezmar@atlas.sk>
Copyright: GPL
Group: Libraries
Packager: Oron Peled <oron@actcom.co.il>
Summary: OpenGUI is a high-Level C/C++ graphics library
Source0: %name-%ver-core.tgz
Source1: OpenGUI-oron.tar.gz
Patch0: OpenGUI-%{ver}-oron.patch
URL: http://www.tutok.sk/fastgl/
BuildRoot: /tmp/buildroot-%name-%ver

%package devel
Summary: development tools for the OpenGUI C/C++ graphics library
Group: Development/Libraries
Requires: %name = %version

%description
OpenGUI is a high-Level C/C++ graphics library built upon a fast, low-level x86
asm graphics kernel. OpenGUI provides 2D drawing primitives and an event-driven
windowing API for easy application development, and it supports the BMP image
file format. It's very powerful, but very easy to use. You can write apps
in the old Borland BGI style or in a windowed style like QT. OpenGUI supports
the keyboard and mouse as event sources, and the Linux framebuffer/svgalib
as drawing backends. Mesa3D is also supported under Linux.
On now are available only 8, 15, 16 and 32-bit color modes.

%description devel
The %name-devel package contains the static libraries and header files
needed for development with %name.

%prep
%setup -n OpenGUI
%setup -n OpenGUI/src -D -a 1

%build
mv makefile makefile.orig
./configure --prefix=%prefix
make

%install
[ "$RPM_BUILD_ROOT" != "/" ] && [ -d $RPM_BUILD_ROOT ] && rm -rf $RPM_BUILD_ROOT
mkdir $RPM_BUILD_ROOT
make prefix=$RPM_BUILD_ROOT%prefix install

%post
PATH="$PATH:/sbin" ldconfig -n %{prefix}/lib

%postun
PATH="$PATH:/sbin" ldconfig -n %{prefix}/lib

%files
%defattr(-, root, root)
%{prefix}/lib/*.so.*
%doc AUTHORS ChangeLog COPYING  INSTALL NEWS README

%files devel
%defattr(-, root, root)
%{prefix}/lib/*.a
%{prefix}/lib/*.so
%{prefix}/include/*

%clean
#[ "$RPM_BUILD_ROOT" != "/" ] && [ -d $RPM_BUILD_ROOT ] && rm -rf $RPM_BUILD_ROO
T

