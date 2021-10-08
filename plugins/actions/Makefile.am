
AM_CPPFLAGS = \
	-DGLIB_DISABLE_DEPRECATION_WARNINGS \
	-I$(top_srcdir) \
	-DG_LOG_DOMAIN=\"libactions\" \
	$(PLATFORM_CPPFLAGS)

plugindir = $(libdir)/xfce4/bar/plugins

plugin_LTLIBRARIES = \
	libactions.la

libactions_built_sources = \
	actions-dialog_ui.h

libactions_la_SOURCES = \
	$(libactions_built_sources) \
	actions.c \
	actions.h

libactions_la_CFLAGS = \
	$(GTK_CFLAGS) \
	$(LIBBLADEUTIL_CFLAGS) \
	$(LIBBLADEUI_CFLAGS) \
	$(BLXO_CFLAGS) \
	$(BLCONF_CFLAGS) \
	$(DBUS_CFLAGS) \
	$(PLATFORM_CFLAGS)

libactions_la_LDFLAGS = \
	-avoid-version \
	-module \
	-no-undefined \
	-export-symbols-regex '^blade_bar_module_(preinit|init|construct)' \
	$(PLATFORM_LDFLAGS)

libactions_la_LIBADD = \
	$(top_builddir)/libbladebar/libbladebar-$(LIBBLADEBAR_VERSION_API).la \
	$(top_builddir)/common/libbar-common.la \
	$(GTK_LIBS) \
	$(LIBBLADEUTIL_LIBS) \
	$(LIBBLADEUI_LIBS) \
	$(BLXO_LIBS) \
	$(DBUS_LIBS) \
	$(BLCONF_LIBS)

libactions_la_DEPENDENCIES = \
	$(top_builddir)/libbladebar/libbladebar-$(LIBBLADEBAR_VERSION_API).la \
	$(top_builddir)/common/libbar-common.la

#
# .desktop file
#
desktopdir = $(datadir)/xfce4/bar/plugins
desktop_in_files = actions.desktop.in.in
desktop_DATA = $(desktop_in_files:.desktop.in.in=.desktop)
@INTLTOOL_DESKTOP_RULE@

EXTRA_DIST = \
	actions-dialog.glade \
	$(desktop_in_files)

DISTCLEANFILES = \
	$(desktop_DATA)

if MAINTAINER_MODE
BUILT_SOURCES = \
	$(libactions_built_sources)

DISTCLEANFILES += \
	$(libactions_built_sources)

actions-dialog_ui.h: actions-dialog.glade
	$(AM_V_GEN) blxo-csource --static --strip-comments --strip-content --name=actions_dialog_ui $< >$@
endif

# vi:set ts=8 sw=8 noet ai nocindent syntax=automake:
