
AM_CPPFLAGS = \
	-DGLIB_DISABLE_DEPRECATION_WARNINGS \
	-I$(top_srcdir) \
	-DG_LOG_DOMAIN=\"libdirectorymenu\" \
	$(PLATFORM_CPPFLAGS)

plugindir = $(libdir)/xfce4/bar/plugins

plugin_LTLIBRARIES = \
	libdirectorymenu.la

libdirectorymenu_built_sources = \
	directorymenu-dialog_ui.h

libdirectorymenu_la_SOURCES = \
	$(libdirectorymenu_built_sources) \
	directorymenu.c \
	directorymenu.h

libdirectorymenu_la_CFLAGS = \
	$(GTK_CFLAGS) \
	$(BLXO_CFLAGS) \
	$(BLCONF_CFLAGS) \
	$(LIBBLADEUTIL_CFLAGS) \
	$(LIBBLADEUI_CFLAGS) \
	$(GIO_CFLAGS) \
	$(GIO_UNIX_CFLAGS) \
	$(PLATFORM_CFLAGS)

libdirectorymenu_la_LDFLAGS = \
	-avoid-version \
	-module \
	-no-undefined \
	-export-symbols-regex '^blade_bar_module_(preinit|init|construct)' \
	$(PLATFORM_LDFLAGS)

libdirectorymenu_la_LIBADD = \
	$(top_builddir)/libbladebar/libbladebar-$(LIBBLADEBAR_VERSION_API).la \
	$(top_builddir)/common/libbar-common.la \
	$(BLXO_LIBS) \
	$(GTK_LIBS) \
	$(LIBBLADEUTIL_LIBS) \
	$(LIBBLADEUI_LIBS) \
	$(GIO_LIBS) \
	$(GIO_UNIX_LIBS) \
	$(BLCONF_LIBS)

libdirectorymenu_la_DEPENDENCIES = \
	$(top_builddir)/libbladebar/libbladebar-$(LIBBLADEBAR_VERSION_API).la \
	$(top_builddir)/common/libbar-common.la

#
# xfce4-popup-directorymenu script
#
bin_SCRIPTS = \
	xfce4-popup-directorymenu

xfce4-popup-directorymenu: xfce4-popup-directorymenu.sh Makefile
	$(AM_V_GEN) $(SED) -e "s,\@bindir\@,$(bindir),g" \
	-e "s,\@localedir\@,$(localedir),g" $< >$@

#
# .desktop file
#
desktopdir = $(datadir)/xfce4/bar/plugins
desktop_in_files = directorymenu.desktop.in.in
desktop_DATA = $(desktop_in_files:.desktop.in.in=.desktop)
@INTLTOOL_DESKTOP_RULE@

EXTRA_DIST = \
	directorymenu-dialog.glade \
	xfce4-popup-directorymenu.sh \
	$(desktop_in_files)

DISTCLEANFILES = \
	$(desktop_DATA) \
	xfce4-popup-directorymenu

if MAINTAINER_MODE
BUILT_SOURCES = \
	$(libdirectorymenu_built_sources)

DISTCLEANFILES += \
	$(libdirectorymenu_built_sources)

directorymenu-dialog_ui.h: directorymenu-dialog.glade
	$(AM_V_GEN) blxo-csource --static --strip-comments --strip-content --name=directorymenu_dialog_ui $< >$@
endif

# vi:set ts=8 sw=8 noet ai nocindent syntax=automake:
