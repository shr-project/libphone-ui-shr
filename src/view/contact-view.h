#ifndef _CONTACT_VIEW_H
#define _CONTACT_VIEW_H

#include <glib.h>
#include "ui-utils.h"

struct ContactViewData {
	struct View parent;
	Evas_Object *pager, *pager_layout;
	Evas_Object *fields, *name, *number, *photo;
	Evas_Object *btn_call, *btn_sms, *btn_delete, *btn_addfield;
	Evas_Object *btn_save, *btn_cancel;
	char *path;
	int entryid;
	GHashTable *properties, *changes;
};

struct ContactFieldData {
	char *type;
	char *name;
	char *value;
	Evas_Object *field_button, *value_entry, *slide_buttons;
	Elm_Genlist_Item *item;
	struct ContactViewData *view;
/*temporary*/
	Evas_Object *edit_widget;
	int isnew;
/* hack because the signals from edje don't work */
	int edit_on;
};


int contact_view_init(char *path, GHashTable *options);
int contact_view_is_init(const char *path);
void contact_view_deinit(struct ContactViewData *view);
void contact_view_show(const char *path);
void contact_view_hide(const char *path);

#endif
