
AM_CPPFLAGS = \
	-DGLIB_DISABLE_DEPRECATION_WARNINGS \
	-I$(top_srcdir) \
	-DG_LOG_DOMAIN=\"libshowdesktop\" \
	-DWNCK_I_KNOW_THIS_IS_UNSTABLE \
	$(PLATFORM_CPPFLAGS)

plugindir = $(libdir)/xfce4/bar/plugins

plugin_LTLIBRARIES = \
	libshowdesktop.la

libshowdesktop_la_SOURCES = \
	showdesktop.c \
	showdesktop.h

libshowdesktop_la_CFLAGS = \
	$(GTK_CFLAGS) \
	$(LIBBLADEUTIL_CFLAGS) \
	$(LIBWNCK_CFLAGS) \
	$(PLATFORM_CFLAGS)

libshowdesktop_la_LDFLAGS = \
	-avoid-version \
	-module \
	-no-undefined \
	-export-symbols-regex '^blade_bar_module_(preinit|init|construct)' \
	$(PLATFORM_LDFLAGS)

libshowdesktop_la_LIBADD = \
	$(top_builddir)/libbladebar/libbladebar-$(LIBBLADEBAR_VERSION_API).la \
	$(top_builddir)/common/libbar-common.la \
	$(GTK_LIBS) \
	$(LIBBLADEUTIL_LIBS) \
	$(LIBWNCK_LIBS)

libshowdesktop_la_DEPENDENCIES = \
	$(top_builddir)/libbladebar/libbladebar-$(LIBBLADEBAR_VERSION_API).la \
	$(top_builddir)/common/libbar-common.la

#
# .desktop file
#
desktopdir = $(datadir)/xfce4/bar/plugins
desktop_in_files = showdesktop.desktop.in.in
desktop_DATA = $(desktop_in_files:.desktop.in.in=.desktop)
@INTLTOOL_DESKTOP_RULE@

EXTRA_DIST = \
	$(desktop_in_files)

DISTCLEANFILES = \
	$(desktop_DATA)

# vi:set ts=8 sw=8 noet ai nocindent syntax=automake:
