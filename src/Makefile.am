MAINTAINERCLEANFILES = Makefile.in

AM_CPPFLAGS = -I$(top_srcdir)/src/util -I$(top_srcdir)/src/widget -I$(top_srcdir)/src/view -DDATADIR=\"$(datadir)\" -DPKGDATADIR=\"$(pkgdatadir)\" -DPACKAGE=\"$(PACKAGE)\" -DLOCALEDIR=\"$(localedir)\" @E_CFLAGS@ @LIBFRAMEWORKD_GLIB_CFLAGS@ @LIBFRAMEWORKD_PHONEGUI_CFLAGS@ @GLIB_CFLAGS@ @LIBPHONE_UTILS_CFLAGS@ -g3

lib_LTLIBRARIES = libframeworkd-phonegui-efl2.la
libframeworkd_phonegui_efl2_la_LIBADD = @LIBFRAMEWORKD_GLIB_LIBS@ @LIBFRAMEWORKD_PHONEGUI_LIBS@ @LIBPHONE_UTILS_LIBS@ @GLIB_LIBS@ @E_LIBS@ @LTLIBINTL@ -lpthread
libframeworkd_phonegui_efl2_la_LDFLAGS = -version-info 0:0:0 -no-undefined
libframeworkd_phonegui_efl2dir = $(includedir)/frameworkd-phonegui-efl2
libframeworkd_phonegui_efl2_HEADERS = 

libframeworkd_phonegui_efl2_la_SOURCES = \
	phonegui-init.h phonegui-init.c \
	phonegui-dialer.h phonegui-dialer.c \
	phonegui-dialog.h phonegui-dialog.c \
	phonegui-messages.h phonegui-messages.c \
	phonegui-incoming-message.h phonegui-incoming-message.c \
	phonegui-sim-auth.h phonegui-sim-auth.c \
	phonegui-call.h phonegui-call.c \
	phonegui-contacts.h phonegui-contacts.c \
	phonegui-contact.h phonegui-contact.c \
	phonegui-ussd.h phonegui-ussd.c \
	widget/etk_tree_model_edje.h widget/etk_tree_model_edje.c \
	widget/elm_keypad.h widget/elm_keypad.c \
	util/helper.h util/helper.c \
	util/instance.h util/instance.c \
	util/window.h util/window.c \
	util/view.h util/view.c \
	view/views.h \
	view/dialer-view.c \
	view/dialog-view.c \
	view/contact-list-common.h \
	view/contact-list-common.c \
	view/contact-show-view.c \
	view/contact-delete-view.c \
	view/contact-list-view.c \
	view/message-new-view.c \
	view/message-delete-view.c \
	view/message-show-view.c \
	view/message-list-view.c \
	view/message-folder-view.c \
	view/call-common.h view/call-common.c \
	view/call-incoming-view.c \
	view/call-active-view.c \
	view/sim-auth-input-view.c \
	view/ussd-view.c

