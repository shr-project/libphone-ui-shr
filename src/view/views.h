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



#define CONTACT_NAME_UNDEFINED_STRING D_("(unknown)")
#define CONTACT_PHONE_UNDEFINED_STRING D_("(no number)")

#define APP_DATA_DIR DATADIR"/libphone-ui-shr"
#define EDJE_DIR APP_DATA_DIR
#define IDLE_SCREEN_THEME EDJE_DIR"/idle_screen.edj"
#define WIDGETS_EDJE EDJE_DIR "/widgets.edj"

#define DELETE_TEXT_ICON APP_DATA_DIR"/edit-undo.png"
#define CONTACT_DEFAULT_PHOTO APP_DATA_DIR"/contact.png"
#define CONTACT_NUMBER_PHOTO APP_DATA_DIR"/number-icon.png"
#define ICON_CALL APP_DATA_DIR"/icon-call.png"
#define ICON_SMS APP_DATA_DIR"/icon-sms.png"
#define ICON_CALL_ACCEPT APP_DATA_DIR"/call-start.png"
#define ICON_CALL_REJECT APP_DATA_DIR"/call-stop.png"

void *dialog_view_show(struct Window *win, void *_options);
void dialog_view_hide(void *_data);

void call_incoming_view_hide(struct CallIncomingViewData *data);
struct CallActiveViewData *call_active_view_show(struct Window *win,
						 GHashTable * options);
struct CallIncomingViewData *call_incoming_view_show(struct Window *win,
						     GHashTable * options);
void call_active_view_hide(struct CallActiveViewData *data);

void *contact_delete_view_show(struct Window *win, void *_options);
void contact_delete_view_hide(void *_data);

void *ussd_view_show(struct Window *win, void *_options);
void ussd_view_hide(void *_data);

#endif
