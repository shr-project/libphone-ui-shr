/*
 *  Copyright (C) 2009, 2010
 *      Authors (alphabetical) :
 *		Tom "TAsn" Hacohen <tom@stosb.com>
 *		Klaus 'mrmoku' Kurzmann <mok@fluxnetz.de>
 *		Martin Jansa <Martin.Jansa@gmail.com>
 *		David Kozub <zub@linux.fjfi.cvut.cz>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301
 */



#include <Elementary.h>
#include <phoneui/phoneui.h>
#include <phoneui/phoneui-utils.h>
#include <phoneui/phoneui-utils-contacts.h>
#include <phoneui/phoneui-utils-calls.h>
#include <phoneui/phoneui-info.h>

#include <glib.h>

#include "views.h"
#include "common-utils.h"
#include "ui-utils.h"
#include "ui-utils-contacts.h"
#include "contact-list-common.h"
#include "phoneui-shr.h"

struct ContactListViewData {
	struct View view;
	struct ContactListData list_data;
	Evas_Object *bt1, *bt2, *bt_options, *bt_message, *bt_edit, *bt_delete;
	Evas_Object *inwin;
};
static struct ContactListViewData view;

static void _list_new_clicked(void *data, Evas_Object *obj, void *event_info);
static void _list_call_number_callback(const char *number, void *data);
static void _list_call_clicked(void *data, Evas_Object *obj, void *event_info);
static void _list_list_longpressed(void *data, Evas_Object *obj, void *event_info);
static void _list_message_number_callback(const char *number, void *data);
static void _list_message_clicked(void *data, Evas_Object *obj, void *event_info);
static void _list_edit_clicked(void *data, Evas_Object *obj, void *event_info);
static void _list_delete_clicked(void *data, Evas_Object *obj, void *event_info);
static void _ctx_dismissed(void *data, Evas_Object *obj, void *event_info);
static void _contact_changed_cb(void *data, const char *path, enum PhoneuiInfoChangeType type);
static void _hide_cb(struct View *view);
static void _delete_cb(struct View *data, Evas_Object *obj, void *event_info);

int
contact_list_view_init()
{
	Evas_Object *win;
	int ret;

	g_debug("Initializing the contact list view");

	ret = ui_utils_view_init(VIEW_PTR(view), ELM_WIN_BASIC, D_("Contacts"),
				 NULL, _hide_cb, NULL);
	if (ret) {
		g_critical("Failed to init the contact list view");
		return ret;
	}

	view.list_data.view = VIEW_PTR(view);
	win = ui_utils_view_window_get(VIEW_PTR(view));
	ui_utils_view_delete_callback_set(VIEW_PTR(view), _delete_cb);
	ui_utils_view_layout_set(VIEW_PTR(view), phoneui_theme, "phoneui/contacts/list");
	elm_theme_extension_add(NULL, phoneui_theme);
	view.list_data.layout = view.view.layout;
	contact_list_add(&view.list_data);
	evas_object_smart_callback_add(view.list_data.list, "longpressed",
				       _list_list_longpressed, win);

	view.bt1 = elm_button_add(win);
	elm_object_text_set(view.bt1, D_("Call"));
	evas_object_smart_callback_add(view.bt1, "clicked",
				       _list_call_clicked, NULL);
	ui_utils_view_swallow(VIEW_PTR(view), "button_new", view.bt1);
	evas_object_show(view.bt1);

	view.bt2 = elm_button_add(win);
	elm_object_text_set(view.bt2, D_("SMS"));
	evas_object_smart_callback_add(view.bt2, "clicked",
				       _list_message_clicked, NULL);
	ui_utils_view_swallow(VIEW_PTR(view), "button_call", view.bt2);
	evas_object_show(view.bt2);

	view.bt_options = elm_button_add(win);
	elm_object_text_set(view.bt_options, D_("New"));
	evas_object_smart_callback_add(view.bt_options, "clicked",
				       _list_new_clicked, NULL);
	ui_utils_view_swallow(VIEW_PTR(view), "button_options", view.bt_options);
	evas_object_show(view.bt_options);


	contact_list_fill(&view.list_data);

	phoneui_info_register_contact_changes(_contact_changed_cb, NULL);

	return 0;
}

int
contact_list_view_is_init()
{
	return ui_utils_view_is_init(VIEW_PTR(view));
}

void
contact_list_view_deinit()
{
	ui_utils_view_deinit(VIEW_PTR(view));
}

void
contact_list_view_show()
{
	ui_utils_view_show(VIEW_PTR(view));
}

void
contact_list_view_hide()
{
	ui_utils_view_hide(VIEW_PTR(view));
}



static void
_list_new_clicked(void *data, Evas_Object * obj, void *event_info)
{
	(void) data;
	(void) obj;
	(void) event_info;
	phoneui_contacts_contact_new(NULL);
}

static void
_list_call_number_callback(const char *number, void *data)
{
	(void) data;
	g_debug("_list_call_number_callback: %s", number ? number : "NO NUMBER");
	if (number) {
		phoneui_utils_dial(number, NULL, NULL);
	}
}

static void
_list_call_clicked(void *data, Evas_Object * obj, void *event_info)
{
	(void) data;
	(void) obj;
	(void) event_info;
	Elm_Object_Item *glit;
	GHashTable *properties;
	const Eina_List *contacts;

	contacts = elm_genlist_selected_items_get(view.list_data.list);
	if( !contacts || eina_list_count(contacts) > 1 )
		return;

	glit = eina_list_data_get(contacts);
	properties = glit ? (GHashTable *) elm_object_item_data_get(glit) : NULL;
	if (properties) {
		GVariant *tmp;
		tmp = g_hash_table_lookup(properties, "Path");
		if (tmp) {
			const char *path = g_variant_get_string(tmp, NULL);
			ui_utils_contacts_contact_number_select(VIEW_PTR(view),
				path, _list_call_number_callback, NULL);
		}
	}
}

static void
_list_list_longpressed(void *data, Evas_Object *obj, void *event_info)
{
	(void) data;
	(void) obj;
	(void) event_info;
	Evas_Object *ctx;
	Evas_Coord x, y;

	ctx = elm_ctxpopup_add(view.list_data.list);
	evas_object_smart_callback_add(ctx, "dismissed", _ctx_dismissed, NULL);
	elm_ctxpopup_item_append(ctx, D_("Edit"), NULL, _list_edit_clicked, event_info);
	elm_ctxpopup_item_append(ctx, D_("Delete"), NULL, _list_delete_clicked, event_info);
	evas_pointer_canvas_xy_get(evas_object_evas_get(obj), &x, &y);
	elm_genlist_item_selected_set((Elm_Object_Item *)event_info, EINA_TRUE);
	evas_object_move(ctx, x, y);
	evas_object_show(ctx);
}

static void
_list_message_number_callback(const char *number, void *data)
{
	(void) data;
	GVariant *tmp;
	char *str;
	GHashTable *properties = data;

	if (!number)
		return;

	GHashTable *options = g_hash_table_new_full(g_str_hash, g_str_equal,
						NULL, common_utils_variant_unref);
	g_hash_table_insert(options, "Phone",
			g_variant_ref_sink(g_variant_new_string(number)));

	str = phoneui_utils_contact_display_name_get(properties);
	if (str) {
		g_hash_table_insert(options, "Name",
			g_variant_ref_sink(g_variant_new_string(str)));
		free(str);
	}
	/*FIXME: make sure it works */
	tmp = g_hash_table_lookup(properties, "Photo");
	if (tmp) {
		g_hash_table_insert(options, "Photo", g_variant_ref(tmp));
	}

	phoneui_messages_message_new(options);
	g_hash_table_unref(options);
}

static void
_list_message_clicked(void *data, Evas_Object * obj, void *event_info)
{
	(void) data;
	(void) obj;
	(void) event_info;
	Elm_Object_Item *glit;
	GHashTable *properties;
	const Eina_List *contacts;

	// FIXME: messages should support multiple destinations, but for now, they don't
	contacts = elm_genlist_selected_items_get(view.list_data.list);
	if( !contacts || eina_list_count(contacts) > 1 )
		return;

	glit = eina_list_data_get(contacts);
	properties = glit ? (GHashTable *) elm_object_item_data_get(glit) : NULL;
	if (properties) {
		GVariant *tmp;
		tmp = g_hash_table_lookup(properties, "Path");
		if (tmp) {
			const char *path = g_variant_get_string(tmp, NULL);
			ui_utils_contacts_contact_number_select(VIEW_PTR(view),
				path, _list_message_number_callback, properties);
		}
	}
}

static void
_list_edit_clicked(void *data, Evas_Object * obj, void *event_info)
{
	(void) obj;
	(void) event_info;
	GHashTable *properties;

	g_debug("editing selected contact");
	evas_object_del(obj);

	if (!data)
		return;

	properties = elm_object_item_data_get(data);
	if (properties) {
		GVariant *tmp;
		tmp = g_hash_table_lookup(properties, "Path");
		if (tmp) {
			g_debug("with path %s", g_variant_get_string(tmp, NULL));
			phoneui_contacts_contact_show(g_variant_get_string(tmp, NULL));
		} else
			g_warning("NO PATH for selected contact?!");
	}
}


static void
_contact_delete_confirm_cb(int result, void *data)
{
	GHashTable *properties=NULL;
	Elm_Object_Item *contact = (Elm_Object_Item *)data;

	if (result != DIALOG_YES)
		return;

	properties = (GHashTable *) elm_object_item_data_get(contact);
	if (properties) {
		GVariant *tmp;
		tmp = g_hash_table_lookup(properties, "Path");
		if(tmp) {
			g_debug("with path %s", g_variant_get_string(tmp, NULL));
			phoneui_utils_contact_delete(g_variant_get_string(tmp, NULL), NULL, NULL);
		} else
			g_warning("NO PATH for selected contact?!");
		// TODO: use a callback to show success/failure
	}
}

static void
_list_delete_clicked(void *data, Evas_Object * obj, void *event_info)
{
	(void) obj;
	(void) event_info;

	evas_object_del(obj);

	if (!data)
		return;

	ui_utils_dialog(VIEW_PTR(view), D_("Really delete contact details?"), 
			DIALOG_YES|DIALOG_NO, _contact_delete_confirm_cb, data);
	// FIXME: this should say what contact is being deleted, now...
	//EINA_LIST_FOREACH(contacts, c, contact)
	//	ui_utils_dialog(VIEW_PTR(view), D_("Really delete contact details?"),
	//			DIALOG_YES|DIALOG_NO, _contact_delete_confirm_cb, contact);
}

static void
_ctx_dismissed(void *data, Evas_Object *obj, void *event_info)
{
	(void) data;
	(void) obj;
	(void) event_info;

	g_debug("ctx popup dismissed");

	evas_object_del(obj);
}

static void
_add_contact_cb(GError *error, GHashTable *properties, gpointer data)
{
	(void) data;
	Elm_Object_Item *glit;
	if (error || !properties) {
		if (error)
			g_warning("Failed adding contact: (%d) %s",
				error->code, error->message);
		else
			g_warning("Failed adding contact: call succeeded, "
				"but no properties were returned");
		ui_utils_error_message_from_gerror_show(VIEW_PTR(view),
			D_("Failed adding contact."), error);
		return;
	}
	g_debug("Adding contact to the list");
	glit = contact_list_item_add(&view.list_data, properties, 1);
	if (glit) {
		elm_genlist_item_bring_in(glit, ELM_GENLIST_ITEM_SCROLLTO_IN);
	}
}

static void
_add_contact(const char *path)
{
	phoneui_utils_contact_get(path, _add_contact_cb, NULL);
}

static void
_remove_contact(const char *path)
{
	Elm_Object_Item *glit;
	GHashTable *properties;
	GVariant *tmp;

	g_debug("Removing contact %s from list", path);
	glit = elm_genlist_first_item_get(view.list_data.list);
	while (glit) {
		properties = (GHashTable *)elm_object_item_data_get(glit);
		tmp = g_hash_table_lookup(properties, "Path");
		if (tmp) {
			if (!strcmp(path, g_variant_get_string(tmp, NULL))) {
				g_debug("found him - removing");
				elm_object_item_del(glit);
				break;
			}
		}
		glit = elm_genlist_item_next_get(glit);
	}
}

static void
_contact_changed_cb(void *data, const char *path, enum PhoneuiInfoChangeType type)
{
	g_debug("contact %s got changed", path);
	(void)data;
	switch (type) {
	case PHONEUI_INFO_CHANGE_UPDATE:
		_remove_contact(path);
		_add_contact(path);
		break;
	case PHONEUI_INFO_CHANGE_NEW:
		_add_contact(path);
		break;
	case PHONEUI_INFO_CHANGE_DELETE:
		_remove_contact(path);
		break;
	}
	contact_list_fill_index(&view.list_data);
	ui_utils_view_swallow(VIEW_PTR(view), "index", view.list_data.index);
}

static void
_hide_cb(struct View *view)
{
	elm_genlist_item_bring_in(elm_genlist_first_item_get(
		((struct ContactListViewData *)view)->list_data.list), ELM_GENLIST_ITEM_SCROLLTO_IN);
}

static void
_delete_cb(struct View *view, Evas_Object *obj, void *event_info)
{
	(void)view;
	(void)obj;
	(void)event_info;
	contact_list_view_hide();
}
