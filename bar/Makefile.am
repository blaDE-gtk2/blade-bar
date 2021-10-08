
AM_CPPFLAGS = \
	-DGLIB_DISABLE_DEPRECATION_WARNINGS \
	-I$(top_srcdir) \
	-DG_LOG_DOMAIN=\"blade-bar\" \
	-DWNCK_I_KNOW_THIS_IS_UNSTABLE \
	-DDATADIR=\"$(datadir)/xfce4\" \
	-DLIBDIR=\"$(libdir)/xfce4\" \
	-DHELPERDIR=\"$(HELPER_PATH_PREFIX)/xfce4/bar\" \
	-DPACKAGE_LOCALE_DIR=\"$(localedir)\" \
	-DDBUS_API_SUBJECT_TO_CHANGE \
	-DWNCK_I_KNOW_THIS_IS_UNSTABLE \
	$(PLATFORM_CPPFLAGS)

bin_PROGRAMS = \
	blade-bar

blade_bar_built_sources = \
	bar-dbus-service-infos.h \
	bar-dbus-client-infos.h \
	bar-marshal.c \
	bar-marshal.h \
	bar-plugin-external-wrapper-infos.h \
	bar-preferences-dialog-ui.h

blade_bar_SOURCES = \
	$(blade_bar_built_sources) \
	main.c \
	bar-application.c \
	bar-application.h \
	bar-base-window.c \
	bar-base-window.h \
	bar-dbus-client.c \
	bar-dbus-client.h \
	bar-dbus-service.c \
	bar-dbus-service.h \
	bar-dialogs.c \
	bar-dialogs.h \
	bar-item-dialog.c \
	bar-item-dialog.h \
	bar-itembar.c \
	bar-itembar.h \
	bar-module.c \
	bar-module.h \
	bar-module-factory.c \
	bar-module-factory.h \
	bar-plugin-external.c \
	bar-plugin-external.h \
	bar-plugin-external-wrapper.c \
	bar-plugin-external-wrapper.h \
	bar-plugin-external-46.c \
	bar-plugin-external-46.h \
	bar-preferences-dialog.c \
	bar-preferences-dialog.h \
	bar-tic-tac-toe.c \
	bar-tic-tac-toe.h \
	bar-window.c \
	bar-window.h

blade_bar_CFLAGS = \
	$(GTK_CFLAGS) \
	$(GMODULE_CFLAGS) \
	$(BLXO_CFLAGS) \
	$(LIBBLADEUTIL_CFLAGS) \
	$(LIBBLADEUI_CFLAGS) \
	$(LIBWNCK_CFLAGS) \
	$(DBUS_CFLAGS) \
	$(BLCONF_CFLAGS) \
	$(LIBX11_CFLAGS) \
	$(LIBWNCK_CFLAGS) \
	$(PLATFORM_CFLAGS)

blade_bar_LDFLAGS = \
	-no-undefined \
	$(PLATFORM_LDFLAGS)

blade_bar_LDADD = \
	$(top_builddir)/libbladebar/libbladebar-$(LIBBLADEBAR_VERSION_API).la \
	$(top_builddir)/common/libbar-common.la \
	$(GTK_LIBS) \
	$(BLXO_LIBS) \
	$(GMODULE_LIBS) \
	$(LIBBLADEUTIL_LIBS) \
	$(LIBBLADEUI_LIBS) \
	$(LIBWNCK_LIBS) \
	$(BLCONF_LIBS) \
	$(DBUS_LIBS) \
	$(LIBX11_LIBS) \
	$(LIBWNCK_LIBS) \
	-lm

blade_bar_DEPENDENCIES = \
	$(top_builddir)/libbladebar/libbladebar-$(LIBBLADEBAR_VERSION_API).la \
	$(top_builddir)/common/libbar-common.la

if MAINTAINER_MODE

bar-marshal.h: bar-marshal.list Makefile
	$(AM_V_GEN) glib-genmarshal --prefix=_bar_marshal --internal --header $< > $@

bar-marshal.c: bar-marshal.list Makefile
	$(AM_V_GEN) echo "#include <bar/bar-marshal.h>" > $@ \
	&& glib-genmarshal --prefix=_bar_marshal --body $< >> $@

bar-dbus-service-infos.h: $(srcdir)/bar-dbus-service-infos.xml Makefile
	$(AM_V_GEN) dbus-binding-tool --prefix=bar_dbus_service --mode=glib-server $< > $@

bar-dbus-client-infos.h: $(srcdir)/bar-dbus-service-infos.xml Makefile
	$(AM_V_GEN) dbus-binding-tool --mode=glib-client $< > $@

bar-plugin-external-wrapper-infos.h: $(srcdir)/bar-plugin-external-wrapper-infos.xml Makefile
	$(AM_V_GEN) dbus-binding-tool --prefix=bar_plugin_external_wrapper --mode=glib-server $< > $@

bar-preferences-dialog-ui.h: $(srcdir)/bar-preferences-dialog.glade Makefile
	$(AM_V_GEN) blxo-csource --static --strip-comments --strip-content --name=bar_preferences_dialog_ui $< >$@

#bar-marshal.h: $(srcdir)/bar-marshal.list Makefile
#	$(AM_V_GEN) glib-genmarshal --header --prefix=bar_marshal $< > $@
#
#bar-marshal.c: $(srcdir)/bar-marshal.list Makefile
#	$(AM_V_GEN) echo "#include <bar/bar-marshal.h>" > $@ \
#	&& glib-genmarshal --body --prefix=bar_marshal $< >> $@

DISTCLEANFILES = \
	$(blade_bar_built_sources)

BUILT_SOURCES = \
	$(blade_bar_built_sources)

endif

EXTRA_DIST = \
	bar-dbus-service-infos.xml \
	bar-plugin-external-wrapper-infos.xml \
	bar-preferences-dialog.glade \
	bar-marshal.list

# vi:set ts=8 sw=8 noet ai nocindent syntax=automake:
