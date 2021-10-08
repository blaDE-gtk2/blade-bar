
AM_CPPFLAGS = \
	-DGLIB_DISABLE_DEPRECATION_WARNINGS \
	-I$(top_srcdir) \
	-DG_LOG_DOMAIN=\"libbladebar\" \
	-DLIBBLADEBAR_COMPILATION \
	-DPACKAGE_LOCALE_DIR=\"$(localedir)\" \
	$(PLATFORM_CPPFLAGS)

lib_LTLIBRARIES = \
	libbladebar-1.0.la

libbladebar_public_built_sources = \
	libbladebar-enum-types.h

libbladebar_built_sources = \
	$(libbladebar_public_built_sources) \
	libbladebar-alias.h \
	libbladebar-aliasdef.c \
	libbladebar-enum-types.c \
	libbladebar-marshal.c \
	libbladebar-marshal.h

libbladebar_headers = \
	libbladebar.h \
	libbladebar-config.h \
	libbladebar-enums.h \
	xfce-arrow-button.h \
	blade-bar-convenience.h \
	blade-bar-macros.h \
	blade-bar-macros-46.h \
	blade-bar-plugin.h \
	blade-bar-plugin-provider.h \
	blade-bar-image.h

libbladebar_includedir = \
	$(includedir)/xfce4/libbladebar-$(LIBBLADEBAR_VERSION_API)/libbladebar

libbladebar_include_HEADERS = \
	$(libbladebar_public_built_sources) \
	$(libbladebar_headers) \
	xfce-hvbox.h

libbladebar_1_0_la_SOURCES = \
	$(libbladebar_built_sources) \
	$(libbladebar_headers) \
	xfce-hvbox.h \
	libbladebar-config.c \
	xfce-arrow-button.c \
	xfce-hvbox.c \
	blade-bar-convenience.c \
	blade-bar-plugin.c \
	blade-bar-plugin-provider.c \
	blade-bar-image.c

libbladebar_1_0_la_CFLAGS =	 \
	$(GTK_CFLAGS) \
	$(LIBBLADEUTIL_CFLAGS) \
	$(PLATFORM_CFLAGS)

libbladebar_1_0_la_LDFLAGS = \
	-no-undefined \
	-export-dynamic \
	-version-info $(LIBBLADEBAR_VERINFO) \
	-export-symbols-regex '^[^_].*' \
	$(PLATFORM_LDFLAGS)

libbladebar_1_0_la_LIBADD = \
	$(GTK_LIBS) \
	$(LIBBLADEUTIL_LIBS) \
	-lm

#
# Pkg-config file
#
pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = libbladebar-$(LIBBLADEBAR_VERSION_API).pc

#
# GTK+ 3 support library
#
if ENABLE_GTK3_LIBRARY

lib_LTLIBRARIES += libbladebar-2.0.la

libbladebar2_includedir = \
	$(includedir)/xfce4/libbladebar-2.0/libbladebar

libbladebar2_include_HEADERS = \
	$(libbladebar_public_built_sources) \
	$(libbladebar_headers)

libbladebar_2_0_la_SOURCES = \
	$(libbladebar_built_sources) \
	$(libbladebar_headers) \
	libbladebar-config.c \
	xfce-arrow-button.c \
	blade-bar-convenience.c \
	blade-bar-plugin.c \
	blade-bar-plugin-provider.c \
	blade-bar-image.c

libbladebar_2_0_la_CFLAGS =	 \
	$(GTK3_CFLAGS) \
	$(LIBBLADEUTIL_CFLAGS) \
	$(PLATFORM_CFLAGS)

libbladebar_2_0_la_LDFLAGS = \
	-no-undefined \
	-export-dynamic \
	-version-info $(LIBBLADEBAR_VERINFO) \
	-export-symbols-regex '^[^_].*' \
	$(PLATFORM_LDFLAGS)

libbladebar_2_0_la_LIBADD = \
	$(GTK3_LIBS) \
	$(LIBBLADEUTIL_LIBS) \
	-lm

pkgconfig_DATA += libbladebar-2.0.pc

endif

#
# Build sources
#
if MAINTAINER_MODE

libbladebar-marshal.h: libbladebar-marshal.list Makefile
	$(AM_V_GEN) glib-genmarshal --prefix=_libbladebar_marshal --internal --header $< > $@

libbladebar-marshal.c: libbladebar-marshal.list Makefile
	$(AM_V_GEN) echo "#include <libbladebar/libbladebar-marshal.h>" > $@ \
	&& glib-genmarshal --prefix=_libbladebar_marshal --body $< >> $@

libbladebar-alias.h: make-libbladebar-alias.pl libbladebar.symbols
	$(AM_V_GEN) $(PERL) $(srcdir)/make-libbladebar-alias.pl $(srcdir)/libbladebar.symbols > $@

libbladebar-aliasdef.c: make-libbladebar-alias.pl libbladebar.symbols
	$(AM_V_GEN) $(PERL) $(srcdir)/make-libbladebar-alias.pl -def < $(srcdir)/libbladebar.symbols > $@

libbladebar-enum-types.h: $(libbladebar_headers) Makefile
	$(AM_V_GEN) ( cd $(srcdir) && glib-mkenums \
		--fhead "#ifndef __LIBBLADEBAR_ENUM_TYPES_H__\n#define __LIBBLADEBAR_ENUM_TYPES_H__\n#include <glib-object.h>\nG_BEGIN_DECLS\n" \
		--fprod "/* enumerations from \"@filename@\" */\n" \
		--vhead "GType @enum_name@_get_type (void) G_GNUC_CONST;\n#define XFCE_TYPE_@ENUMSHORT@ (@enum_name@_get_type())\n" \
		--ftail "G_END_DECLS\n\n#endif /* !__LIBBLADEBAR_ENUM_TYPES_H__ */" \
		$(libbladebar_headers) ) > $@

libbladebar-enum-types.c:  $(libbladebar_headers) Makefile
	$(AM_V_GEN) ( cd $(srcdir) && glib-mkenums \
		--fhead "#include <libbladebar/libbladebar-enums.h>\n#include <libbladebar/libbladebar-enum-types.h>\n#include <libbladebar/libbladebar-alias.h>" \
		--fprod "\n/* enumerations from \"@filename@\" */" \
		--vhead "GType\n@enum_name@_get_type (void)\n{\n\tstatic GType type = 0;\n\tif (type == 0) {\n\tstatic const G@Type@Value values[] = {"\
		--vprod "\t{ @VALUENAME@, \"@VALUENAME@\", \"@valuenick@\" }," \
		--vtail "\t{ 0, NULL, NULL }\n\t};\n\ttype = g_@type@_register_static (\"@EnumName@\", values);\n  }\n\treturn type;\n}\n" \
		--ftail "\n#define __LIBBLADEBAR_ENUM_TYPES_C__\n#include <libbladebar/libbladebar-aliasdef.c>\n" \
		$(libbladebar_headers) ) > $@

DISTCLEANFILES = \
	$(libbladebar_built_sources)

BUILT_SOURCES = \
	$(libbladebar_built_sources)

CLEANFILES = \
	actual-abi \
	expected-abi

endif

EXTRA_DIST = \
	abicheck.sh \
	libbladebar.symbols \
	libbladebar-alias.h \
	libbladebar-aliasdef.c \
	libbladebar-marshal.list \
	make-libbladebar-alias.pl

if HAVE_GNUC_VISIBILITY
TESTS =	abicheck.sh
endif

# required for gtk-doc
dist-hook: all

# vi:set ts=8 sw=8 noet ai nocindent syntax=automake:
