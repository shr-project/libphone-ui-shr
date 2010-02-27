MAINTAINERCLEANFILES = Makefile.in

AM_CFLAGS = -I$(top_srcdir)/src/util -I$(top_srcdir)/src/widget -I$(top_srcdir)/src/view \
	      -DDATADIR=\"$(datadir)\" -DPKGDATADIR=\"$(pkgdatadir)\" -DPACKAGE=\"$(PACKAGE)\" \
	      -DLOCALEDIR=\"$(localedir)\" -DG_LOG_DOMAIN=\"libphone-ui-shr\" \
	      @E_CFLAGS@ @LIBPHONEUI_CFLAGS@ \
	      @GLIB_CFLAGS@ @LIBPHONE_UTILS_CFLAGS@ -g3 -Wall -Wextra -Werror

pkgdir		= $(libdir)/phoneui/modules
pkg_LTLIBRARIES	= shr.la
shr_la_LIBADD = @LIBPHONEUI_LIBS@ @LIBPHONE_UTILS_LIBS@ @GLIB_LIBS@ @E_LIBS@ @LTLIBINTL@ -lpthread
shr_la_LDFLAGS = -module -avoid-version

shr_la_SOURCES = \
	phoneui-init.h phoneui-init.c \
	phoneui-dialer.h phoneui-dialer.c \
	phoneui-idle.h phoneui-idle.c \
	phoneui-dialog.h phoneui-dialog.c \
	phoneui-messages.h phoneui-messages.c \
	phoneui-sim-auth.h phoneui-sim-auth.c \
	phoneui-call.h phoneui-call.c \
	phoneui-contacts.h phoneui-contacts.c \
	phoneui-ussd.h phoneui-ussd.c \
	phoneui-quick-settings.h phoneui-quick-settings.c \
	phoneui-phonelog.h phoneui-phonelog.c \
	widget/elm_keypad.h widget/elm_keypad.c \
	util/window.h util/window.c \
	util/ui-utils.h util/ui-utils.c \
	view/views.h \
	view/dialer-view.c \
	view/idle-view.c \
	view/dialog-view.c \
	view/contact-list-common.h \
	view/contact-list-common.c \
	view/contact-view.c \
	view/contact-view.h \
	view/contact-delete-view.c \
	view/contact-list-view.c \
	view/message-new-view.c \
	view/message-delete-view.c \
	view/message-show-view.c \
	view/message-list-view.c \
	view/call-common.h view/call-common.c \
	view/call-incoming-view.c \
	view/call-active-view.c \
	view/sim-auth-input-view.c \
	util/common-utils.h util/common-utils.c \
	view/ussd-view.c \
	view/quick-settings-view.h \
	view/quick-settings-view.c \
	view/phonelog-view.h \
	view/phonelog-view.c

