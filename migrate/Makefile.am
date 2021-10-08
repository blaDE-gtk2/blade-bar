
AM_CPPFLAGS = \
	-DGLIB_DISABLE_DEPRECATION_WARNINGS \
	-I$(top_srcdir) \
	-DG_LOG_DOMAIN=\"blade-bar-migrate\" \
	-DPACKAGE_LOCALE_DIR=\"$(localedir)\" \
	-DXDGCONFIGDIR=\"$(sysconfdir)/xdg\" \
	$(PLATFORM_CPPFLAGS)

confdir = $(sysconfdir)/xdg/xfce4/bar
conf_DATA = \
	default.xml

migratedir = $(HELPER_PATH_PREFIX)/xfce4/bar

migrate_PROGRAMS = \
	migrate

migrate_SOURCES = \
	main.c \
	migrate-46.c \
	migrate-46.h \
	migrate-config.c \
	migrate-config.h \
	migrate-default.c \
	migrate-default.h

migrate_CFLAGS = \
	$(GTK_CFLAGS) \
	$(BLCONF_CFLAGS) \
	$(LIBBLADEUTIL_CFLAGS) \
	$(LIBBLADEUI_CFLAGS) \
	$(PLATFORM_CFLAGS)

migrate_LDFLAGS = \
	-no-undefined \
	$(PLATFORM_LDFLAGS)

migrate_LDADD = \
	$(top_builddir)/libbladebar/libbladebar-$(LIBBLADEBAR_VERSION_API).la \
	$(GTK_LIBS) \
	$(BLCONF_LIBS) \
	$(LIBBLADEUTIL_LIBS) \
	$(LIBBLADEUI_LIBS)

migrate_DEPENDENCIES = \
	$(top_builddir)/libbladebar/libbladebar-$(LIBBLADEBAR_VERSION_API).la

EXTRA_DIST = \
	default.xml

# vi:set ts=8 sw=8 noet ai nocindent syntax=automake:
