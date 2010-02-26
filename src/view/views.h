#ifndef _VIEWS_H
#define _VIEWS_H

#include <dbus/dbus-glib.h>
#include <glib.h>
#include <phoneui/phoneui.h>
#include <Ecore_Evas.h>
#include <Ecore.h>
#include <Edje.h>
#include <stdlib.h>
#include "window.h"
#include "call-common.h"
#include "contact-list-common.h"

/* MOVE to a better place */
#define LTR_STRING "\xE2\x80\x8E"

#define D_(String) dgettext(PACKAGE, String)

#define CONTACT_NAME_UNDEFINED_STRING D_("(unknown)")
#define CONTACT_PHONE_UNDEFINED_STRING D_("(no number)")

#define DEFAULT_THEME DATADIR"/libphone-ui-shr/default.edj"
#define IDLE_SCREEN_THEME DATADIR"/libphone-ui-shr/idle_screen.edj"

#define DELETE_TEXT_ICON DATADIR"/libphone-ui-shr/edit-undo.png"
#define CONTACT_DEFAULT_PHOTO DATADIR"/libphone-ui-shr/contact.png"
#define CONTACT_NUMBER_PHOTO DATADIR"/libphone-ui-shr/number-icon.png"
#define ICON_CALL DATADIR"/libphone-ui-shr/icon-call.png"
#define ICON_SMS DATADIR"/libphone-ui-shr/icon-sms.png"
#define ICON_CALL_ACCEPT DATADIR"/libphone-ui-shr/call-start.png"
#define ICON_CALL_REJECT DATADIR"/libphone-ui-shr/call-stop.png"

void *dialog_view_show(struct Window *win, void *_options);
void dialog_view_hide(void *_data);

void *message_new_view_show(struct Window *win, void *_options);
void message_new_view_hide(void *_data);

void *message_folder_view_show(struct Window *win, void *_options);
void message_folder_view_hide(void *_data);

void *message_show_view_show(struct Window *win, void *_options);
void message_show_view_hide(void *_data);

void *message_delete_view_show(struct Window *win, void *_options);
void message_delete_view_hide(void *_data);

void *message_list_view_show(struct Window *win, void *_options);
void message_list_view_hide(void *_data);

void *sim_auth_input_view_show(struct Window *win, void *_options);
void sim_auth_input_view_hide(void *_data);

void call_incoming_view_hide(struct CallIncomingViewData *data);
struct CallActiveViewData *call_active_view_show(struct Window *win,
						 GHashTable * options);
struct CallIncomingViewData *call_incoming_view_show(struct Window *win,
						     GHashTable * options);
void call_active_view_hide(struct CallActiveViewData *data);

void *contact_list_view_show(struct Window *win, void *_options);
void contact_list_view_hide(void *_data);
void contact_list_view_refresh(struct Window *win);

void *contact_show_view_show(struct Window *win, void *_options);
void contact_show_view_hide(void *_data);

void *contact_delete_view_show(struct Window *win, void *_options);
void contact_delete_view_hide(void *_data);

void *ussd_view_show(struct Window *win, void *_options);
void ussd_view_hide(void *_data);

#endif
