
AM_CPPFLAGS = \
	-DGLIB_DISABLE_DEPRECATION_WARNINGS \
	-I$(top_srcdir) \
	-DG_LOG_DOMAIN=\"libbar-common\" \
	-DLIBBLADEBAR_COMPILATION \
	-DHELPDIR=\"$(docdir)/html\" \
	$(PLATFORM_CPPFLAGS)

noinst_LTLIBRARIES = \
	libbar-common.la

libbar_common_la_SOURCES = \
	bar-debug.c \
	bar-debug.h \
	bar-utils.c \
	bar-utils.h \
	bar-blconf.c \
	bar-blconf.h

libbar_common_la_CFLAGS = \
	$(BLCONF_CFLAGS) \
	$(GTK_CFLAGS) \
	$(BLXO_CFLAGS) \
	$(LIBBLADEUI_CFLAGS) \
	$(PLATFORM_CFLAGS)

libbar_common_la_LDFLAGS = \
	-no-undefined \
	$(PLATFORM_LDFLAGS)

libbar_common_la_LIBADD = \
	$(BLCONF_LIBS) \
	$(GTK_LIBS) \
	$(LIBBLADEUI_LIBS) \
	$(BLXO_LIBS)

EXTRA_DIST = \
	bar-dbus.h \
	bar-private.h

# vi:set ts=8 sw=8 noet ai nocindent syntax=automake:
