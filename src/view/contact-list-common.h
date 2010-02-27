
#ifndef _CONTACTLIST_COMMON_H
#define _CONTACTLIST_COMMON_H

#include "ui-utils.h"

struct ContactListViewData {
	struct View view;
	Evas_Object *list;
	Evas_Object *index;
	Evas_Object *bx, *hv;
	Evas_Object *bt1, *bt2, *bt_options, *bt_message, *bt_edit, *bt_delete;
	Evas_Object *inwin;
	int contact_count, contact_current;
};

void contact_list_fill_index(struct ContactListViewData *data);
void contact_list_fill(struct ContactListViewData *data);
void contact_list_add(struct ContactListViewData *data);
Elm_Genlist_Item *contact_list_item_add(struct ContactListViewData *data, GHashTable *entry, int sortin);


#endif
