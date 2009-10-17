MAINTAINERCLEANFILES = Makefile.in

AM_CPPFLAGS = -I$(top_srcdir)/src/util -I$(top_srcdir)/src/widget -I$(top_srcdir)/src/view -DDATADIR=\"$(datadir)\" -DPKGDATADIR=\"$(pkgdatadir)\" -DPACKAGE=\"$(PACKAGE)\" -DLOCALEDIR=\"$(localedir)\" @E_CFLAGS@ @LIBFRAMEWORKD_GLIB_CFLAGS@ @LIBPHONEUI_CFLAGS@ @GLIB_CFLAGS@ @LIBPHONE_UTILS_CFLAGS@ -g3

pkgdir		= $(libdir)/phoneui/modules
pkg_LTLIBRARIES	= shr.la
shr_la_LIBADD = @LIBFRAMEWORKD_GLIB_LIBS@ @LIBPHONEUI_LIBS@ @LIBPHONE_UTILS_LIBS@ @GLIB_LIBS@ @E_LIBS@ @LTLIBINTL@ -lpthread
shr_la_LDFLAGS = -module -avoid-version

shr_la_SOURCES = \
	phoneui-init.h phoneui-init.c \
	phoneui-dialer.h phoneui-dialer.c \
	phoneui-dialog.h phoneui-dialog.c \
	phoneui-messages.h phoneui-messages.c \
	phoneui-incoming-message.h phoneui-incoming-message.c \
	phoneui-sim-auth.h phoneui-sim-auth.c \
	phoneui-call.h phoneui-call.c \
	phoneui-contacts.h phoneui-contacts.c \
	phoneui-ussd.h phoneui-ussd.c \
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
	view/common-utils.c \
	view/ussd-view.c

