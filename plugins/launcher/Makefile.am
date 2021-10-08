
AM_CPPFLAGS = \
	-DGLIB_DISABLE_DEPRECATION_WARNINGS \
	-I$(top_srcdir) \
	-DG_LOG_DOMAIN=\"liblauncher\" \
	-DSN_API_NOT_YET_FROZEN \
	$(PLATFORM_CPPFLAGS)

plugindir = $(libdir)/xfce4/bar/plugins

plugin_LTLIBRARIES = \
	liblauncher.la

liblauncher_built_sources = \
	launcher-dialog_ui.h

liblauncher_la_SOURCES = \
	$(liblauncher_built_sources) \
	launcher.c \
	launcher.h \
	launcher-dialog.c \
	launcher-dialog.h

liblauncher_la_CFLAGS = \
	$(GTK_CFLAGS) \
	$(LIBBLADEUTIL_CFLAGS) \
	$(LIBBLADEUI_CFLAGS) \
	$(POJK_CFLAGS) \
	$(BLXO_CFLAGS) \
	$(GIO_CFLAGS) \
	$(BLCONF_CFLAGS) \
	$(DBUS_CFLAGS) \
	$(PLATFORM_CFLAGS)

liblauncher_la_LDFLAGS = \
	-avoid-version \
	-module \
	-no-undefined \
	-export-symbols-regex '^blade_bar_module_(preinit|init|construct)' \
	$(PLATFORM_LDFLAGS)

liblauncher_la_LIBADD = \
	$(top_builddir)/libbladebar/libbladebar-$(LIBBLADEBAR_VERSION_API).la \
	$(top_builddir)/common/libbar-common.la \
	$(GTK_LIBS) \
	$(LIBBLADEUTIL_LIBS) \
	$(LIBBLADEUI_LIBS) \
	$(POJK_LIBS) \
	$(BLXO_LIBS) \
	$(DBUS_LIBS) \
	$(GIO_LIBS) \
	$(BLCONF_LIBS)

liblauncher_la_DEPENDENCIES = \
	$(top_builddir)/libbladebar/libbladebar-$(LIBBLADEBAR_VERSION_API).la \
	$(top_builddir)/common/libbar-common.la

#
# .desktop file
#
desktopdir = $(datadir)/xfce4/bar/plugins
desktop_in_files = launcher.desktop.in.in
desktop_DATA = $(desktop_in_files:.desktop.in.in=.desktop)
@INTLTOOL_DESKTOP_RULE@

EXTRA_DIST = \
	launcher-dialog.glade \
	$(desktop_in_files)

DISTCLEANFILES = \
	$(desktop_DATA)

if MAINTAINER_MODE
BUILT_SOURCES = \
	$(liblauncher_built_sources)

DISTCLEANFILES += \
	$(libclock_built_sources)

launcher-dialog_ui.h: launcher-dialog.glade
	$(AM_V_GEN) blxo-csource --static --strip-comments --strip-content --name=launcher_dialog_ui $< >$@
endif

# vi:set ts=8 sw=8 noet ai nocindent syntax=automake:
