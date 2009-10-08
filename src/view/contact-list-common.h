
#ifndef _CONTACTLIST_COMMON_H
#define _CONTACTLIST_COMMON_H

#include <dbus/dbus-glib.h>
#include <glib.h>
#include <Ecore_Evas.h>
#include <Ecore.h>
#include <Edje.h>
#include <Etk.h>
#include <stdlib.h>
#include <assert.h>
#include "helper.h"
#include "window.h"



struct ContactListViewData {
	struct Window *win;
	Evas_Object *list;
	Evas_Object *index;
	Evas_Object *bx, *hv;
	Evas_Object *bt1, *bt2, *bt_options, *bt_message, *bt_edit, *bt_delete;
	Evas_Object *inwin;
	DBusGProxy *query;
	char *current_index;
	Elm_Genlist_Item *current_index_item;
	GPtrArray *contacts;
	int contact_count;
	int index_count;
	int new_index;
};


void
  contact_list_fill(struct ContactListViewData *data);

Evas_Object *contact_list_add(struct ContactListViewData *data);


#endif // _CONTACTS_COMMON_H
