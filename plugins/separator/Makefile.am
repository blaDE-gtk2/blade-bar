
AM_CPPFLAGS = \
	-DGLIB_DISABLE_DEPRECATION_WARNINGS \
	-I$(top_srcdir) \
	-DG_LOG_DOMAIN=\"libseparator\" \
	$(PLATFORM_CPPFLAGS)

plugindir = $(libdir)/xfce4/bar/plugins

plugin_LTLIBRARIES = \
	libseparator.la

libseparator_built_sources = \
	separator-dialog_ui.h

libseparator_la_SOURCES = \
	$(libseparator_built_sources) \
	separator.c \
	separator.h

libseparator_la_CFLAGS = \
	$(GTK_CFLAGS) \
	$(BLCONF_CFLAGS) \
	$(LIBBLADEUTIL_CFLAGS) \
	$(CAIRO_CFLAGS) \
	$(LIBBLADEUI_CFLAGS) \
	$(BLXO_CFLAGS) \
	$(PLATFORM_CFLAGS)

libseparator_la_LDFLAGS = \
	-avoid-version \
	-module \
	-no-undefined \
	-export-symbols-regex '^blade_bar_module_(preinit|init|construct)' \
	$(PLATFORM_LDFLAGS)

libseparator_la_LIBADD = \
	$(top_builddir)/libbladebar/libbladebar-$(LIBBLADEBAR_VERSION_API).la \
	$(top_builddir)/common/libbar-common.la \
	$(GTK_LIBS) \
	$(CAIRO_LIBS) \
	$(LIBBLADEUTIL_LIBS) \
	$(LIBBLADEUI_LIBS) \
	$(BLXO_LIBS) \
	$(BLCONF_LIBS)

libseparator_la_DEPENDENCIES = \
	$(top_builddir)/libbladebar/libbladebar-$(LIBBLADEBAR_VERSION_API).la \
	$(top_builddir)/common/libbar-common.la

#
# .desktop file
#
desktopdir = $(datadir)/xfce4/bar/plugins
desktop_in_files = separator.desktop.in.in
desktop_DATA = $(desktop_in_files:.desktop.in.in=.desktop)
@INTLTOOL_DESKTOP_RULE@

EXTRA_DIST = \
	separator-dialog.glade \
	$(desktop_in_files)

DISTCLEANFILES = \
	$(desktop_DATA)

if MAINTAINER_MODE
BUILT_SOURCES = \
	$(libseparator_built_sources)

DISTCLEANFILES += \
	$(libseparator_built_sources)

separator-dialog_ui.h: separator-dialog.glade
	$(AM_V_GEN) blxo-csource --static --strip-comments --strip-content --name=separator_dialog_ui $< >$@
endif

# vi:set ts=8 sw=8 noet ai nocindent syntax=automake:
