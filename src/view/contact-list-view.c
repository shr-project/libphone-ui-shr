
#include <Elementary.h>
#include <phoneui/phoneui.h>
#include <phoneui/phoneui-utils.h>
#include <phoneui/phoneui-utils-contacts.h>
#include <phoneui/phoneui-utils-calls.h>
#include <phoneui/phoneui-info.h>

#include <glib.h>
#include <dbus/dbus-glib.h>

#include "views.h"
#include "common-utils.h"
#include "ui-utils.h"
#include "ui-utils-contacts.h"
#include "contact-list-common.h"
#include "phoneui-shr.h"

struct ContactListViewData {
	struct View view;
	struct ContactListData list_data;
	Evas_Object *bx, *hv;
	Evas_Object *bt1, *bt2, *bt_options, *bt_message, *bt_edit, *bt_delete;
	Evas_Object *inwin;
};
static struct ContactListViewData view;

static void _list_new_clicked(void *data, Evas_Object *obj, void *event_info);
static void _list_call_number_callback(const char *number, void *data);
static void _list_call_clicked(void *data, Evas_Object *obj, void *event_info);
static void _list_options_clicked(void *data, Evas_Object *obj, void *event_info);
static void _list_message_number_callback(const char *number, void *data);
static void _list_message_clicked(void *data, Evas_Object *obj, void *event_info);
static void _list_edit_clicked(void *data, Evas_Object *obj, void *event_info);
static void _list_delete_clicked(void *data, Evas_Object *obj, void *event_info);
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
        elm_theme_extension_add(phoneui_theme);
	view.list_data.layout = view.view.layout;
	contact_list_add(&view.list_data);

	view.bt1 = elm_button_add(win);
	elm_button_label_set(view.bt1, D_("New"));
	evas_object_smart_callback_add(view.bt1, "clicked",
				       _list_new_clicked, NULL);
	ui_utils_view_swallow(VIEW_PTR(view), "button_new", view.bt1);
	evas_object_show(view.bt1);

	view.bt2 = elm_button_add(win);
	elm_button_label_set(view.bt2, D_("Call"));
	evas_object_smart_callback_add(view.bt2, "clicked",
				       _list_call_clicked, NULL);
	ui_utils_view_swallow(VIEW_PTR(view), "button_call", view.bt2);
	evas_object_show(view.bt2);

	view.bt_options = elm_button_add(win);
	elm_button_label_set(view.bt_options, D_("Options"));
	evas_object_smart_callback_add(view.bt_options, "clicked",
				       _list_options_clicked, NULL);
	ui_utils_view_swallow(VIEW_PTR(view), "button_options", view.bt_options);
	evas_object_show(view.bt_options);

	/* Options */
	view.hv = elm_hover_add(win);
	elm_hover_parent_set(view.hv, win);
	elm_hover_target_set(view.hv, view.bt_options);

	view.bx = elm_box_add(win);
	elm_box_horizontal_set(view.bx, 0);
	elm_box_homogenous_set(view.bx, 1);
	evas_object_show(view.bx);

	view.bt_message = elm_button_add(win);
	elm_button_label_set(view.bt_message, D_("SMS"));
	evas_object_size_hint_min_set(view.bt_message, 130, 80);
	evas_object_smart_callback_add(view.bt_message, "clicked",
				       _list_message_clicked, NULL);
	evas_object_show(view.bt_message);
	elm_box_pack_end(view.bx, view.bt_message);

	view.bt_edit = elm_button_add(win);
	elm_button_label_set(view.bt_edit, D_("Show"));
	evas_object_size_hint_min_set(view.bt_edit, 130, 80);
	evas_object_smart_callback_add(view.bt_edit, "clicked",
				       _list_edit_clicked, NULL);
	evas_object_show(view.bt_edit);
	elm_box_pack_end(view.bx, view.bt_edit);

	view.bt_delete = elm_button_add(win);
	elm_button_label_set(view.bt_delete, D_("Delete"));
	evas_object_size_hint_min_set(view.bt_delete, 130, 80);
	evas_object_smart_callback_add(view.bt_delete, "clicked",
				       _list_delete_clicked, NULL);
	evas_object_show(view.bt_delete);
	elm_box_pack_end(view.bx, view.bt_delete);

	elm_hover_content_set(view.hv, "top", view.bx);

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
	evas_object_hide(view.hv);
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
	Elm_Genlist_Item *it;
	GHashTable *properties;

	it = elm_genlist_selected_item_get(view.list_data.list);
	properties = it ? (GHashTable *) elm_genlist_item_data_get(it) : NULL;
	if (properties) {
		GValue *gval_tmp;
		gval_tmp = g_hash_table_lookup(properties, "Path");
		if (gval_tmp) {
			const char *path = g_value_get_string(gval_tmp);
			ui_utils_contacts_contact_number_select(VIEW_PTR(view),
				path, _list_call_number_callback, NULL);
		}
	}
}

static void
_list_options_clicked(void *data, Evas_Object * obj, void *event_info)
{
	(void) data;
	(void) obj;
	(void) event_info;
	evas_object_show(view.hv);
}

static void
_list_message_number_callback(const char *number, void *data)
{
	(void) data;
	GValue *gval_tmp;
	char *str;
	const char *cstr;
	GHashTable *properties = data;

	if (!number)
		return;

	GHashTable *options = g_hash_table_new_full(g_str_hash, g_str_equal,
						NULL, common_utils_gvalue_free);
	g_hash_table_insert(options, "Phone",
			common_utils_new_gvalue_string(number));

	str = phoneui_utils_contact_display_name_get(properties);
	if (str) {
		g_hash_table_insert(options, "Name",
			common_utils_new_gvalue_string(str));
		free(str);
	}
	/*FIXME: make sure it works */
	gval_tmp = g_hash_table_lookup(properties, "Photo");
	if (gval_tmp) {
		cstr = g_value_get_string(gval_tmp);
		g_hash_table_insert(options, "Photo",
			common_utils_new_gvalue_string(str));
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
	Elm_Genlist_Item *it;
	GHashTable *properties;

	evas_object_hide(view.hv);

	it = elm_genlist_selected_item_get(view.list_data.list);
	properties = it ? (GHashTable *) elm_genlist_item_data_get(it) : NULL;
	if (properties) {
		GValue *gval_tmp;
		gval_tmp = g_hash_table_lookup(properties, "Path");
		if (gval_tmp) {
			const char *path = g_value_get_string(gval_tmp);
			ui_utils_contacts_contact_number_select(VIEW_PTR(view),
				path, _list_message_number_callback, properties);
		}
	}
}

static void
_list_edit_clicked(void *data, Evas_Object * obj, void *event_info)
{
	(void) data;
	(void) obj;
	const Elm_Genlist_Item *it;

	evas_object_hide(view.hv);
	if (event_info) {
		it = (Elm_Genlist_Item *) event_info;
	}
	else {
		it = elm_genlist_selected_item_get(view.list_data.list);
	}
	GHashTable *properties = it ? (GHashTable *) elm_genlist_item_data_get(it) : NULL;
	if (properties != NULL) {
		GValue *tmp;
		tmp = g_hash_table_lookup(properties, "Path");
		if (tmp) {
			phoneui_contacts_contact_show(g_value_get_string(tmp));
		}
	}
}


static void
_contact_delete_confirm_cb(int result, void *data)
{
	if (result != DIALOG_YES)
		return;

	Elm_Genlist_Item *it = (Elm_Genlist_Item *)data;
	GHashTable *properties = (it) ? (GHashTable *) elm_genlist_item_data_get(it) : NULL;
	if (properties) {

		const char *path = g_value_get_string(
				g_hash_table_lookup(properties, "Path"));
		// TODO: use a callback to show success/failure
		phoneui_utils_contact_delete(path, NULL, NULL);
	}
}

static void
_list_delete_clicked(void *data, Evas_Object * obj, void *event_info)
{
	(void) data;
	(void) obj;
	(void) event_info;
	Elm_Genlist_Item *it;

	evas_object_hide(view.hv);

	it = elm_genlist_selected_item_get(view.list_data.list);
	if (it) {
		ui_utils_dialog(VIEW_PTR(view), D_("Really delete this contact?"),
			DIALOG_YES|DIALOG_NO, _contact_delete_confirm_cb, it);

	}
}

static void
_add_contact_cb(GError *error, GHashTable *properties, gpointer data)
{
	(void) data;
	Elm_Genlist_Item *it;
	if (error || !properties) {
		// FIXME: show some nice notification
		g_warning("Failed adding a contact");
		return;
	}
	g_debug("Adding contact to the list");
	it = contact_list_item_add(&view.list_data, properties, 1);
	if (it) {
		elm_genlist_item_bring_in(it);
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
	Elm_Genlist_Item *it;
	GHashTable *properties;
	GValue *tmp;

	g_debug("Removing contact %s from list", path);
	it = elm_genlist_first_item_get(view.list_data.list);
	while (it) {
		properties = (GHashTable *)elm_genlist_item_data_get(it);
		tmp = g_hash_table_lookup(properties, "Path");
		if (tmp) {
			if (!strcmp(path, g_value_get_string(tmp))) {
				g_debug("found him - removing");
				elm_genlist_item_del(it);
				break;
			}
		}
		it = elm_genlist_item_next_get(it);
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
		((struct ContactListViewData *)view)->list_data.list));
}

static void
_delete_cb(struct View *view, Evas_Object *obj, void *event_info)
{
	(void)view;
	(void)obj;
	(void)event_info;
	contact_list_view_hide();
}
