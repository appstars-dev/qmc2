Name:           qmc2
Version:        0.40
Release:        2
Summary:        M.A.M.E./M.E.S.S./U.M.E. Catalog / Launcher II
Group:          System/Emulators/Other
License:        GPL-2.0
URL:            http://qmc2.arcadehits.net/wordpress
Source0:        http://dl.sourceforge.net/qmc2/%{name}-%{version}.tar.bz2

BuildRequires:  libqt4-devel
BuildRequires:  libkde4-devel
BuildRequires:  libqt4-x11
BuildRequires:  SDL-devel
BuildRequires:  make
BuildRequires:  gcc
BuildRequires:  rsync
BuildRequires:  desktop-file-utils
BuildRequires:  openSUSE-release
BuildRequires:  fdupes
BuildRequires:  man-pages

%description
QMC2 is a Qt 4 based multi-platform GUI front end for several MAME, MESS and UME variants.

%prep
%setup -qcT
tar -xjf %{SOURCE0}
mv %{name} sdlmame
tar -xjf %{SOURCE0}
mv %{name} sdlmess
tar -xjf %{SOURCE0}
mv %{name} sdlume
tar -xjf %{SOURCE0}
mv %{name} arcade
tar -xjf %{SOURCE0}
mv %{name} qchdman
tar -xjf %{SOURCE0}
mv %{name} manpages

%build
pushd sdlmess
make %{?_smp_mflags} QMAKE=%{_prefix}/bin/qmake DISTCFG=1 \
    PREFIX=%{_prefix} SYSCONFDIR=%{_sysconfdir} \
    EMULATOR=SDLMESS JOYSTICK=1 PHONON=1 WIP=0 OPENGL=0 \
    CXX_FLAGS=-O3 CC_FLAGS=-O3
popd

pushd sdlume
make %{?_smp_mflags} QMAKE=%{_prefix}/bin/qmake DISTCFG=1 \
    PREFIX=%{_prefix} SYSCONFDIR=%{_sysconfdir} \
    EMULATOR=SDLUME JOYSTICK=1 PHONON=1 WIP=0 OPENGL=0 \
    CXX_FLAGS=-O3 CC_FLAGS=-O3
popd

pushd sdlmame
make %{?_smp_mflags} QMAKE=%{_prefix}/bin/qmake DISTCFG=1 \
    PREFIX=%{_prefix} SYSCONFDIR=%{_sysconfdir} \
    EMULATOR=SDLMAME JOYSTICK=1 PHONON=1 WIP=0 OPENGL=0 \
    CXX_FLAGS=-O3 CC_FLAGS=-O3
popd

pushd arcade
make %{?_smp_mflags} QMAKE=%{_prefix}/bin/qmake DISTCFG=1 \
    PREFIX=%{_prefix} SYSCONFDIR=%{_sysconfdir} \
    JOYSTICK=1 WIP=0 \
    CXX_FLAGS=-O3 CC_FLAGS=-O3 arcade
popd

pushd qchdman
make %{?_smp_mflags} QMAKE=%{_prefix}/bin/qmake DISTCFG=1 \
    PREFIX=%{_prefix} SYSCONFDIR=%{_sysconfdir} \
    JOYSTICK=1 WIP=0 \
    CXX_FLAGS=-O3 CC_FLAGS=-O3 qchdman
popd

pushd manpages
make %{?_smp_mflags} QMAKE=%{_prefix}/bin/qmake DISTCFG=1 \
    PREFIX=%{_prefix} SYSCONFDIR=%{_sysconfdir} \
    JOYSTICK=1 WIP=0 \
    CXX_FLAGS=-O3 CC_FLAGS=-O3 man
popd

%install
rm -rf $RPM_BUILD_ROOT

pushd sdlmess
make install QMAKE=%{_prefix}/bin/qmake DESTDIR=$RPM_BUILD_ROOT DISTCFG=1 \
    PREFIX=%{_prefix} SYSCONFDIR=%{_sysconfdir} \
    EMULATOR=SDLMESS JOYSTICK=1 PHONON=1 WIP=0 OPENGL=0 \
    CXX_FLAGS=-O3 CC_FLAGS=-O3
popd

rm -f $RPM_BUILD_ROOT%{_sysconfdir}/qmc2/qmc2.ini

pushd sdlume
make install QMAKE=%{_prefix}/bin/qmake DESTDIR=$RPM_BUILD_ROOT DISTCFG=1 \
    PREFIX=%{_prefix} SYSCONFDIR=%{_sysconfdir} \
    EMULATOR=SDLUME JOYSTICK=1 PHONON=1 WIP=0 OPENGL=0 \
    CXX_FLAGS=-O3 CC_FLAGS=-O3
popd

rm -f $RPM_BUILD_ROOT%{_sysconfdir}/qmc2/qmc2.ini

pushd sdlmame
make install QMAKE=%{_prefix}/bin/qmake DESTDIR=$RPM_BUILD_ROOT DISTCFG=1 \
    PREFIX=%{_prefix} SYSCONFDIR=%{_sysconfdir} \
    EMULATOR=SDLMAME JOYSTICK=1 PHONON=1 WIP=0 OPENGL=0 \
    CXX_FLAGS=-O3 CC_FLAGS=-O3
popd

pushd arcade
make arcade-install QMAKE=%{_prefix}/bin/qmake DESTDIR=$RPM_BUILD_ROOT DISTCFG=1 \
    PREFIX=%{_prefix} SYSCONFDIR=%{_sysconfdir} \
    JOYSTICK=1 WIP=0 \
    CXX_FLAGS=-O3 CC_FLAGS=-O3
popd

pushd qchdman
make qchdman-install QMAKE=%{_prefix}/bin/qmake DESTDIR=$RPM_BUILD_ROOT DISTCFG=1 \
    PREFIX=%{_prefix} SYSCONFDIR=%{_sysconfdir} \
    JOYSTICK=1 WIP=0 \
    CXX_FLAGS=-O3 CC_FLAGS=-O3
popd

pushd manpages
make man-install QMAKE=%{_prefix}/bin/qmake DESTDIR=$RPM_BUILD_ROOT DISTCFG=1 \
    PREFIX=%{_prefix} SYSCONFDIR=%{_sysconfdir} \
    JOYSTICK=1 WIP=0 \
    CXX_FLAGS=-O3 CC_FLAGS=-O3
popd

# manually install doc files in order to avoid "files-duplicate" warning
install -dp -m 0755 $RPM_BUILD_ROOT%{_defaultdocdir}/%{name}
cp -a sdlmame/data/doc/html/ $RPM_BUILD_ROOT%{_defaultdocdir}/%{name}/

# symlink duplicate files
%fdupes -s $RPM_BUILD_ROOT/usr/share

# update the desktop files
%suse_update_desktop_file %{name}-sdlmame Game ArcadeGame
%suse_update_desktop_file %{name}-sdlmess Game ArcadeGame
%suse_update_desktop_file %{name}-sdlume Game ArcadeGame
%suse_update_desktop_file %{name}-arcade Game ArcadeGame
%suse_update_desktop_file qchdman Game ArcadeGame

# make sure the executable permissions are set correctly
chmod 755 $RPM_BUILD_ROOT%{_bindir}/qmc2-sdlmame
chmod 755 $RPM_BUILD_ROOT%{_bindir}/qmc2-sdlmess
chmod 755 $RPM_BUILD_ROOT%{_bindir}/qmc2-sdlume
chmod 755 $RPM_BUILD_ROOT%{_bindir}/qmc2-arcade
chmod 755 $RPM_BUILD_ROOT%{_bindir}/runonce
chmod 755 $RPM_BUILD_ROOT%{_bindir}/qchdman

%clean
rm -rf $RPM_BUILD_ROOT

%files
%defattr(-,root,root,-)
%doc %{_defaultdocdir}/%{name}/
%config(noreplace) %{_sysconfdir}/qmc2
%{_bindir}/runonce
%{_datadir}/qmc2
%{_bindir}/qmc2
%{_bindir}/qmc2-sdlmame
%{_bindir}/qmc2-sdlmess
%{_bindir}/qmc2-sdlume
%{_bindir}/qmc2-arcade
%{_bindir}/qchdman
%{_datadir}/applications/qmc2-sdlmame.desktop
%{_datadir}/applications/qmc2-sdlmess.desktop
%{_datadir}/applications/qmc2-sdlume.desktop
%{_datadir}/applications/qmc2-arcade.desktop
%{_datadir}/applications/qchdman.desktop
%{_mandir}/man6/qmc2-main-gui.6.gz
%{_mandir}/man6/qmc2.6.gz
%{_mandir}/man6/qmc2-sdlmame.6.gz
%{_mandir}/man6/qmc2-sdlmess.6.gz
%{_mandir}/man6/qmc2-sdlume.6.gz
%{_mandir}/man6/qmc2-arcade.6.gz
%{_mandir}/man6/qchdman.6.gz
%{_mandir}/man6/runonce.6.gz

%changelog
* Wed Aug 21 2013 R. Reucher <rene[dot]reucher[at]batcom-it[dot]net> - 0.40-2
- added man-page build rules

* Tue Jun 11 2013 R. Reucher <rene[dot]reucher[at]batcom-it[dot]net> - 0.40-1
- updated spec to QMC2 0.40

* Fri Feb 22 2013 R. Reucher <rene[dot]reucher[at]batcom-it[dot]net> - 0.39-2
- added build rules for 'Qt CHDMAN GUI' (qchdman)

* Sat Jan 12 2013 R. Reucher <rene[dot]reucher[at]batcom-it[dot]net> - 0.39-1
- updated spec to QMC2 0.39

* Fri Dec 14 2012 R. Reucher <rene[dot]reucher[at]batcom-it[dot]net> - 0.38-2
- added build rules for 'QMC2 Arcade'

* Mon Sep 17 2012 R. Reucher <rene[dot]reucher[at]batcom-it[dot]net> - 0.38-1
- updated spec to QMC2 0.38

* Tue May 22 2012 R. Reucher <rene[dot]reucher[at]batcom-it[dot]net> - 0.37-1
- updated spec to QMC2 0.37, added new UME emulator target

* Sun Apr 29 2012 R. Reucher <rene[dot]reucher[at]batcom-it[dot]net> - 0.36-1
- updated spec to QMC2 0.36 / added fdupes macro / updated licence name

* Tue Nov 15 2011 R. Reucher <rene[dot]reucher[at]batcom-it[dot]net> - 0.35-1
- updated spec to QMC2 0.35

* Wed Jun 29 2011 R. Reucher <rene[dot]reucher[at]batcom-it[dot]net> - 0.34-1
- updated spec to QMC2 0.34

* Thu Mar 03 2011 R. Reucher <rene[dot]reucher[at]batcom-it[dot]net> - 0.2.b20-1
- updated spec to QMC2 0.2.b20

* Sun Jan 02 2011 R. Reucher <rene[dot]reucher[at]batcom-it[dot]net> - 0.2.b19-1
- updated spec to QMC2 0.2.b19

* Fri Oct 22 2010 R. Reucher <rene[dot]reucher[at]batcom-it[dot]net> - 0.2.b18-1
- updated spec to QMC2 0.2.b18

* Tue Sep 07 2010 R. Reucher <rene[dot]reucher[at]batcom-it[dot]net> - 0.2.b17-1
- initial version using openSUSE's build service
