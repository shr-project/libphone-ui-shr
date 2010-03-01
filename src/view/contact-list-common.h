
#ifndef _CONTACTLIST_COMMON_H
#define _CONTACTLIST_COMMON_H

#include "ui-utils.h"

struct ContactListData {
	struct View *view;
	Evas_Object *list;
	Evas_Object *index;
	int count;
	int current;
};

void contact_list_fill_index(struct ContactListData *list_data);
void contact_list_fill(struct ContactListData *list_data);
void contact_list_add(struct ContactListData *list_data);
Elm_Genlist_Item *contact_list_item_add(struct ContactListData *list_data, GHashTable *entry, int sortin);


#endif
