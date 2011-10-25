/*
 *  Copyright (C) 2009, 2010
 *      Authors (alphabetical) :
 *		Tom "TAsn" Hacohen <tom@stosb.com>
 *		Klaus 'mrmoku' Kurzmann <mok@fluxnetz.de>
 *		David Kozub <zub@linux.fjfi.cvut.cz>
 *		Martin Jansa <Martin.Jansa@gmail.com>
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



#include <glib.h>
#include <glib-object.h>
#include <stdlib.h>
#include <Evas.h>
#include <Elementary.h>
#include <phoneui/phoneui.h>
#include <phoneui/phoneui-utils.h>
#include <phoneui/phoneui-utils-contacts.h>
#include <phoneui/phoneui-utils-calls.h>
#include <phoneui/phoneui-info.h>
#include "views.h"
#include "common-utils.h"
#include "ui-utils.h"
#include "ui-utils-contacts.h"
#include "contact-view.h"
#include "phoneui-shr.h"


static Elm_Genlist_Item_Class itc;
/* keep a list of open contact views - to open only one view per contact */
static GHashTable *contactviews = NULL;

static void _contact_delete_clicked(void *_data, Evas_Object * obj, void *event_info);
static void _contact_add_field_clicked(void *_data, Evas_Object * obj, void *event_info);
static void _contact_photo_clicked(void *_data, Evas_Object * obj, void *event_info);
static void _contact_call_clicked(void *_data, Evas_Object * obj, void *event_info);
static void _contact_call_number_callback(const char *number, void *data);
static void _contact_sms_clicked(void *_data, Evas_Object * obj, void *event_info);
static void _contact_sms_number_callback(const char *number, void *data);

static void _contact_save_clicked(void *_data, Evas_Object *obj, void *event_info);
static void _contact_cancel_clicked(void *_data, Evas_Object *obj, void *event_info);

static void _field_clicked(void *_data, Evas_Object *obj, void *event_info);
static void _field_remove_clicked(void *_data, Evas_Object *obj, void *event_info);
static void _change_value(struct ContactFieldData *fd, const char *value);
static void _load_name(struct ContactViewData *view);
static void _load_number(struct ContactViewData *view);
static void _load_photo(struct ContactViewData *view);
static void _load_fields(struct ContactViewData *view);
static void _add_field(struct ContactViewData *view, const char *key, const char *value, int isnew);

static Evas_Object *gl_field_content_get(void *_data, Evas_Object * obj, const char *part);
static void gl_field_del(void *_data, Evas_Object * obj);

static void _delete_cb(struct View *view, Evas_Object * win, void *event_info);
static void _destroy_cb(struct View *_view);
static void _contact_changed_handler(void *data, int entryid, enum PhoneuiInfoChangeType type);

static void _set_modify(struct ContactViewData *view, int dirty);
static void _update_cb(GError *error, gpointer data);
static void _add_cb(GError *error, char *path, gpointer data);
static void _load_cb(GError *error, GHashTable *content, gpointer data);
static void _field_unselected_cb(void *userdata, Evas_Object *obj, void *event_info);
static Evas_Object *_start_file_selector(Evas_Object *parent, const char *path);

int
contact_view_init(char *path, GHashTable *properties)
{
	struct ContactViewData *view;
	Evas_Object *win;
	int ret;
	GVariant *tmp;

	/* path MUST always be set! For new contacts to ""
	and it will be freed by destroying the contactviews
	hashtable in here, thus must be a copy */
	if (!path) {
		g_warning("Trying to initialize a contact view without path!");
		return 1;
	}

	g_debug("Initializing the contact view for '%s'", path);

	view = malloc(sizeof(struct ContactViewData));
	if (!view) {
		g_critical("Failed to allocate contact view for '%s'", path);
		if (properties) {
			g_hash_table_unref(properties);
		}
		free(path);
		return 1;
	}

	ret = ui_utils_view_init(VIEW_PTR(*view), ELM_WIN_BASIC, D_("Contact"),
				 NULL, NULL, _destroy_cb);
	if (ret) {
		g_critical("Failed to init contact view for '%s'", path);
		if (properties) {
			g_hash_table_unref(properties);
		}
		free(view);
		free(path);
		return ret;
	}

	/* cache the views to open only one view per contact */
	if (contactviews == NULL) {
		contactviews = g_hash_table_new_full(g_str_hash, g_str_equal,
						     free, NULL);
	}
	g_hash_table_insert(contactviews, path, view);

	view->path = path;
	view->entryid = -1;
	if (properties) {
		view->properties = g_hash_table_ref(properties);
		tmp = g_hash_table_lookup(properties, "EntryId");
		if (tmp) {
			view->entryid = g_variant_get_int32(tmp);
			phoneui_info_register_single_contact_changes
				(view->entryid, _contact_changed_handler, view);
		}
	}
	else {
		view->properties = g_hash_table_new_full(g_str_hash, g_str_equal,
						NULL, common_utils_variant_unref);
		g_hash_table_insert(view->properties, "Phone",
				      g_variant_ref_sink(g_variant_new_string("")));
		g_hash_table_insert(view->properties, "Name",
				      g_variant_ref_sink(g_variant_new_string("")));
	}
	view->changes = NULL;

	elm_theme_extension_add(NULL, phoneui_theme);

	win = ui_utils_view_window_get(VIEW_PTR(*view));
	ui_utils_view_delete_callback_set(VIEW_PTR(*view), _delete_cb);

	ui_utils_view_layout_set(VIEW_PTR(*view), phoneui_theme,
				 "phoneui/contacts/view");

	view->photo = elm_icon_add(win);
	evas_object_smart_callback_add(view->photo, "clicked",
				_contact_photo_clicked, view);
	ui_utils_view_swallow(VIEW_PTR(*view), "photo", view->photo);
	evas_object_show(view->photo);

	view->pager = elm_pager_add(win);
	ui_utils_view_swallow(VIEW_PTR(*view), "main", view->pager);

	view->pager_layout = elm_layout_add(win);
	elm_layout_file_set(view->pager_layout, phoneui_theme, "phoneui/contacts/fieldedit");
	evas_object_size_hint_min_set(view->pager_layout, 1, 1);
	view->fields = elm_genlist_add(win);
	elm_scroller_policy_set(view->fields, ELM_SCROLLER_POLICY_OFF,
				ELM_SCROLLER_POLICY_AUTO);
	elm_genlist_horizontal_set(view->fields, ELM_LIST_LIMIT);
	evas_object_size_hint_align_set(view->fields, 0.0, 0.0);
	elm_layout_content_set(view->pager_layout, "fields", view->fields);


	itc.item_style = "contactfield";
	itc.func.label_get = NULL;
	itc.func.content_get = gl_field_content_get;
	itc.func.state_get = NULL;
	itc.func.del = gl_field_del;
	/*FIXME: Shouldn't be these signals,but the start_edit or whatever signa
l contacts.edc emits*/
	evas_object_smart_callback_add(view->fields, "unselected",
		       _field_unselected_cb, NULL);

	evas_object_show(view->fields);

	view->btn_save = elm_button_add(win);
	elm_object_text_set(view->btn_save, D_("Save"));
	evas_object_smart_callback_add(view->btn_save, "clicked",
				       _contact_save_clicked, view);
	elm_layout_content_set(view->pager_layout, "button_save", view->btn_save);
	evas_object_show(view->btn_save);

	view->btn_cancel = elm_button_add(win);
	elm_object_text_set(view->btn_cancel, D_("Cancel"));
	evas_object_smart_callback_add(view->btn_cancel, "clicked",
				       _contact_cancel_clicked, view);
	elm_layout_content_set(view->pager_layout, "button_cancel", view->btn_cancel);
	evas_object_show(view->btn_cancel);

	view->btn_call = elm_button_add(win);
	elm_object_text_set(view->btn_call, D_("Call"));
	evas_object_smart_callback_add(view->btn_call, "clicked",
				       _contact_call_clicked, view);
	elm_layout_content_set(view->pager_layout, "button_call", view->btn_call);
	evas_object_show(view->btn_call);

	view->btn_sms = elm_button_add(win);
	elm_object_text_set(view->btn_sms, D_("SMS"));
	evas_object_smart_callback_add(view->btn_sms, "clicked",
				       _contact_sms_clicked, view);
	elm_layout_content_set(view->pager_layout, "button_sms", view->btn_sms);
	evas_object_show(view->btn_sms);

	view->btn_delete = elm_button_add(win);
	elm_object_text_set(view->btn_delete, D_("Delete"));
	evas_object_smart_callback_add(view->btn_delete, "clicked",
				       _contact_delete_clicked, view);
	evas_object_show(view->btn_delete);
	elm_layout_content_set(view->pager_layout, "button_delete",
			      view->btn_delete);

	view->btn_addfield = elm_button_add(win);
	elm_object_text_set(view->btn_addfield, D_("Add Field"));
	evas_object_smart_callback_add(view->btn_addfield, "clicked",
				       _contact_add_field_clicked, view);
	evas_object_show(view->btn_addfield);
	elm_layout_content_set(view->pager_layout, "button_addfield",
			      view->btn_addfield);

	_load_name(view);
	_load_number(view);
	_load_photo(view);
	_load_fields(view);

	elm_pager_content_push(view->pager, view->pager_layout);
	/* show save and cancel buttons when this is a new one */
	if (!*view->path)
		_set_modify(view, 1);

	return 0;
}

int
contact_view_is_init(const char *path)
{
	struct ContactViewData *view;

	if (contactviews == NULL) {
		return 0;
	}
	view = (struct ContactViewData *)
			g_hash_table_lookup(contactviews, path);
	if (view && ui_utils_view_is_init(VIEW_PTR(*view))) {
		return 1;
	}
	return 0;
}

void
contact_view_deinit(struct ContactViewData *view)
{
	if (view) {
		g_debug("Deiniting view for contact");
		ui_utils_view_deinit(VIEW_PTR(*view));
	}
	else {
		g_warning("contact_view_deinit without a view!");
	}
}

void
contact_view_show(const char *path)
{
	struct ContactViewData *view;

	g_debug("looking up contact view for '%s'", path);

	if (contactviews == NULL) {
		g_debug("No contact views loaded yet");
		return;
	}
	view = (struct ContactViewData *)
			g_hash_table_lookup(contactviews, path);
	if (view) {
		ui_utils_view_show(VIEW_PTR(*view));
	}
	else {
		g_warning("Could not find view for contact '%s'", path);
	}
	g_debug("contact view show done");
}

static void
_update_changes_of_field(struct ContactViewData *view, const char *field, const char *old_value, const char *new_value)
{
	/*FIXME: make generic so it'll be good for all the field types */
	char **value;
	if (!view->changes) {
		view->changes = g_hash_table_new_full(g_str_hash, g_str_equal, free, (void (*) (void *)) g_strfreev);
	}

	value = g_hash_table_lookup(view->changes, field);
	if (!value) {
		GVariant *prop;
		/* If we haven't already changed this field, we should first check the
		 * the property hash to keep the values of the same field */
		prop = g_hash_table_lookup(view->properties, field);
		if (!prop) {
			/* New field */
			value = calloc(1, sizeof(char *));
			value[0] = NULL;
		}
		else {
			if (g_variant_is_of_type(prop, G_VARIANT_TYPE_STRING)) {
				value = calloc(2, sizeof(char *));
				value[0] = g_variant_dup_string(prop, NULL);
				value[1] = NULL;
			}
			else if (g_variant_is_of_type(prop, G_VARIANT_TYPE_STRING_ARRAY)) {
				value = g_variant_dup_strv(prop, NULL);
			}
			else {
				g_debug("We don't handle gvalues that are not boxed/strings yet");
				return;
			}
		}
	}
	else {
		value = g_strdupv(value); /*Copy it to our own, the old one will be erased by the hash table */
	}

	char **cur;


	/* try to find the value we want to remove */
	for (cur = value ; *cur ; cur++) {
		if (!strcmp(old_value, *cur)) {
			break;
		}
	}

	/* If we found what we wanted to remove, replace */
	if (*cur) {
		free(*cur);
		/*If we need to replace, just allocate, otherwise remove that field from list */
		if (strcmp(new_value, "")) {
			*cur = strdup(new_value);
		}
		else {
			/* We removed a field, we should reallocate -1 */
			for ( ; *cur ; cur++) {
				*cur = *(cur + 1);
			}
			value = realloc(value, (g_strv_length(value) + 1)  * sizeof (char *));
		}
	}
	else { /* We need to add another one */
		/* We added a field, we should reallocate +1 */
		int size = g_strv_length(value) + 2;
		value = realloc(value, size * sizeof (char *)); /*One for the NULL as well */
		value[size - 2] = strdup(new_value);
		value[size - 1] = NULL;
	}

	/* This also removes the old value */
	g_hash_table_replace(view->changes, strdup(field), value);
}

static void
_contact_delete_cb(GError *error, gpointer data)
{
	struct ContactViewData *view = (struct ContactViewData *)data;
	if (error) {
		ui_utils_dialog(VIEW_PTR(*view),
				D_("Deleting the contact failed!"), DIALOG_OK,
				NULL, NULL);
		g_warning("Deleting the contact failed: %s", error->message);
		return;
	}
	contact_view_deinit(view);
}

static void
_contact_delete_confirm_cb(int result, void *data)
{
	struct ContactViewData *view = (struct ContactViewData *)data;
	if (result == DIALOG_YES) {
		if (*view->path) {
			phoneui_utils_contact_delete(view->path,
					_contact_delete_cb, view);
			return;
		}
		contact_view_deinit(view);
	}
}

static void
_contact_delete_clicked(void *_data, Evas_Object * obj, void *event_info)
{
	(void) obj;
	(void) event_info;
	struct ContactViewData *view = (struct ContactViewData *)_data;

	ui_utils_dialog(VIEW_PTR(*view), D_("Really delete this contact?"),
			DIALOG_YES|DIALOG_NO, _contact_delete_confirm_cb, view);
}

static void
_add_field_cb(const char *field, void *data)
{
	struct ContactViewData *view = (struct ContactViewData *)data;
	g_debug("selected field '%s'", field);
	if (field) {
		_add_field(view, field, "", 1);
	}
}

static void
_contact_add_field_clicked(void *_data, Evas_Object * obj, void *event_info)
{
	(void) obj;
	(void) event_info;
	struct ContactViewData *view = (struct ContactViewData *)_data;
	ui_utils_contacts_field_select(VIEW_PTR(*view), _add_field_cb, view);
}

static Evas_Object *
_start_file_selector(Evas_Object *parent, const char *path)
{
	Evas_Object *content;
	/*layout = elm_layout_add(view->pager);
	elm_layout_file_set(view->pager_layout, phoneui_theme, "phoneui/contacts/fileselect");
	*/

	content = elm_fileselector_add(parent);
	elm_fileselector_is_save_set(content, EINA_FALSE);
	elm_fileselector_expandable_set(content, EINA_FALSE);
	elm_fileselector_buttons_ok_cancel_set(content, EINA_FALSE);
	elm_fileselector_path_set(content, path);

	evas_object_size_hint_weight_set(content, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_size_hint_align_set(content, EVAS_HINT_FILL, EVAS_HINT_FILL);
	evas_object_show(content);

	return content;
}

static void
_contact_photo_clicked(void *_data, Evas_Object *obj, void *event_info)
{
	(void) obj;
	(void) event_info;
	struct ContactViewData *view = _data;
	(void) view;
	//_start_file_selector(view, getenv("HOME"));
}

static void
_contact_call_number_callback(const char *number, void *data)
{
	(void) data;
// 	struct ContactViewData *view = data;
	if (!number) {
		g_debug("got NO number to dial");
		return;
	}
	g_debug("got number %s to dial", number);
	phoneui_utils_dial(number, NULL, NULL);
}

static void
_contact_call_clicked(void *_data, Evas_Object * obj, void *event_info)
{
	(void) obj;
	(void) event_info;
	struct ContactViewData *view = (struct ContactViewData *)_data;
	if (view->properties == NULL)
		return;
	g_debug("Triggering phone number selection");
	ui_utils_contacts_contact_number_select(VIEW_PTR(*view), view->path,
						_contact_call_number_callback,
						view);
}

static void
_contact_sms_number_callback(const char *number, void *data)
{
	GVariant *tmp;
	char *str;
	const char *cstr;
	struct ContactViewData *view = data;

	if (!number)
		return;

	GHashTable *options = g_hash_table_new_full(g_str_hash, g_str_equal,
						NULL, common_utils_variant_unref);
	g_hash_table_insert(options, "Phone",
			      g_variant_ref_sink(g_variant_new_string(number)));

	str = phoneui_utils_contact_display_name_get(view->properties);
	if (str) {
		g_hash_table_insert(options, "Name",
				g_variant_ref_sink(g_variant_new_string(str)));
		free(str);
	}
	tmp = g_hash_table_lookup(view->properties, "Photo");
	if (tmp) {
		cstr = g_variant_get_string(tmp, NULL);
		g_hash_table_insert(options, "Photo",
				g_variant_ref_sink(g_variant_new_string(cstr)));
	}

	phoneui_messages_message_new(options);
	g_hash_table_unref(options);
}

static void
_contact_sms_clicked(void *_data, Evas_Object * obj, void *event_info)
{
	(void) obj;
	(void) event_info;
	struct ContactViewData *view = _data;

	if (view->properties == NULL)
		return;

	ui_utils_contacts_contact_number_select(VIEW_PTR(*view), view->path,
						_contact_sms_number_callback,
						view);
}

static void
_change_field_type_cb(GError *error, char *type, gpointer data)
{
	struct ContactFieldData *fd = (struct ContactFieldData *)data;
	if (error || !type) {
		type = strdup("generic");
	}
	if (fd->type) {
		free(fd->type);
	}
	fd->type = strdup(type);
	/*FIXME: free type*/
}

static void
_change_field_cb(const char *field, void *data)
{
	g_debug("_change_field_cb");
	struct ContactFieldData *fd = (struct ContactFieldData *)data;
	if (field) {
		_update_changes_of_field(fd->view, fd->name, fd->value, "");
		_update_changes_of_field(fd->view, field, "", fd->value);
		fd->name = strdup(field);
		elm_object_text_set(fd->field_button, field);
		_set_modify(fd->view, 1);
		phoneui_utils_contacts_field_type_get(fd->name, _change_field_type_cb, fd);
	}
}

static void
_field_clicked(void *_data, Evas_Object *obj, void *event_info)
{
	(void) obj;
	(void) event_info;
	struct ContactFieldData *fd = (struct ContactFieldData *)_data;
	ui_utils_contacts_field_select(VIEW_PTR(*fd->view),
				       _change_field_cb, fd);
}

static void
_field_remove_clicked(void *_data, Evas_Object *obj, void *event_info)
{
	(void) obj;
	(void) event_info;
	g_debug("_field_remove_clicked");
	struct ContactFieldData *fd = (struct ContactFieldData *)_data;
	_change_value(fd, "");
	elm_genlist_item_del(fd->item);
}

static void
_change_value(struct ContactFieldData *fd, const char *value)
{
	g_debug("_value_changed");
	if (value) {
		_update_changes_of_field(fd->view, fd->name, fd->value, value);
		if (fd->value) {
			free(fd->value);
		}
		fd->value = strdup(value);
	}
	else {
		g_error("Got NULL from edit field %s", fd->name);
	}
}

static void
_sanitize_changes_hash_foreach(void *key, void *value, void *data)
{
	GHashTable *target = data;
	char **tmp = value;
	GVariant *gtmp;
	if (!tmp) {
		g_warning("%s:%d - Got an empty value in the hash table (key = %s), shouldn't have happend", __FILE__, __LINE__, (char *) key);
	}
	if (!tmp || !*tmp || !**tmp) { /*If the only one we have is empty, don't put in a list */
		gtmp = g_variant_new_string(value);
	}
	else if (g_strv_length(tmp) == 1) {
		gtmp = g_variant_new_string(tmp[0]);
	}
	else {
		gtmp = g_variant_new_strv(value, g_strv_length(value));
	}
	g_hash_table_insert(target, strdup((char *)key), g_variant_ref_sink(gtmp));
}

static GHashTable *
_sanitize_changes_hash(GHashTable *source)
{
	GHashTable *target;
	target = g_hash_table_new_full(g_str_hash, g_str_equal, NULL, common_utils_variant_unref);
	g_hash_table_foreach(source, _sanitize_changes_hash_foreach,target);
	return target;
}

static void
_contact_save_clicked(void *_data, Evas_Object *obj, void *event_info)
{
	(void) obj;
	(void) event_info;
	struct ContactViewData *view = (struct ContactViewData *)_data;
	GHashTable *formatted_changes;
	Elm_Genlist_Item *it;
	struct ContactFieldData *fd;
	char *s;

	/* check if the selected field is still in edit mode */
	it = elm_genlist_selected_item_get(view->fields);
	if (it) {
		fd = (struct ContactFieldData *)elm_genlist_item_data_get(it);
		if (fd->edit_on) {
			s = ui_utils_entry_utf8_get(fd->value_entry);
			_change_value(fd, s);
			free(s);
			elm_entry_editable_set(fd->value_entry, EINA_FALSE);
			fd->edit_on = 0;
		}
	}

	if (!view->changes) {
		/* should not happen ? */
		g_warning("No changes found ?");
		return;
	}

	formatted_changes = _sanitize_changes_hash(view->changes);
	common_utils_debug_dump_hashtable(view->properties);
	common_utils_debug_dump_hashtable(formatted_changes);
	if (*view->path) {
		g_debug("Updating contact '%s'", view->path);
		phoneui_utils_contact_update(view->path, formatted_changes,
					     _update_cb, view);
	}
	else {
		g_debug("Saving a new contact");
		phoneui_utils_contact_add(formatted_changes, _add_cb, view);
	}
	g_hash_table_unref(formatted_changes);
}

static void
_contact_cancel_clicked(void *_data, Evas_Object *obj, void *event_info)
{
	(void) obj;
	(void) event_info;
	struct ContactViewData *view = (struct ContactViewData *)_data;
	_set_modify(view, 0);
	if (*view->path) {
		_load_fields(view);
		return;
	}

	/* for new contacts cancel means closing the view */
	contact_view_deinit(view);
	free(view);
}

static void
_set_modify(struct ContactViewData *view, int dirty)
{
	g_debug("modify is %s now", dirty ? "ON" : "OFF");
	if (dirty) {
		edje_object_signal_emit(
			elm_layout_edje_get(view->pager_layout),
					"elm,state,dirty", "");
	}
	else {
		edje_object_signal_emit(
			elm_layout_edje_get(view->pager_layout),
					"elm,state,default", "");
	}
}

static void
_update_cb(GError *error, gpointer data)
{
	struct ContactViewData *view = data;
	if (error) {
		g_warning("Updating contact %s failed: (%d) %s", view->path,
			error->code, error->message);
		ui_utils_error_message_from_gerror_show(VIEW_PTR(view),
			D_("Updating contact failed."), error);
	}
	else {
		_set_modify(view, 0);
	}
}

static void
_add_cb(GError *error, char *path, gpointer data)
{
	struct ContactViewData *view = data;
	if (error) {
		g_warning("Adding the contact failed: (%d) %s",
			error->code, error->message);
		ui_utils_error_message_from_gerror_show(VIEW_PTR(view),
			D_("Adding the contact failed."), error);
	}
	else {
		view->path = path;
		phoneui_utils_contact_get(view->path, _load_cb, view);
		_set_modify(view, 0);
	}
}

static void
_load_cb(GError *error, GHashTable *content, gpointer data)
{
	struct ContactViewData *view = (struct ContactViewData *)data;
	g_debug("_load_cb called");
	if (error || !content) {
		g_critical("Failed loading data of saved contact: (%d) %s",
			(error)? error->code : 0, (error)? error->message : "NULL");
		ui_utils_error_message_from_gerror_show(VIEW_PTR(view),
			D_("Failed loading data of saved contact."), error);
		return;
	}
	/* cleanup up the old data of the contact */
	if (view->properties) {
		g_hash_table_unref(view->properties);
	}
	if (view->changes) {
		g_hash_table_destroy(view->changes);
		view->changes = NULL;
	}
	view->properties = g_hash_table_ref(content);
	_load_name(view);
	_load_number(view);
	_load_photo(view);
	_load_fields(view);
}


static void
_field_edit_button_back_clicked_cb(void *data, Evas_Object *obj, void *event_info)
{
       (void) obj;
       (void) event_info;
       struct ContactFieldData *fd = data;
       elm_pager_content_pop(fd->view->pager);
}

static void
_field_edit_button_remove_clicked_cb(void *data, Evas_Object *obj, void *event_info)
{
       (void) obj;
       (void) event_info;
       struct ContactFieldData *fd = data;
       elm_pager_content_pop(fd->view->pager);
       _change_value(fd, "");
       elm_genlist_item_del(fd->item);
}

static void
_field_edit_add_edit_page(struct ContactFieldData *fd, Evas_Object *content,
		void (*save_cb) (void *, Evas_Object *, void *))
{
	Evas_Object *layout;
	Evas_Object *btn_back, *btn_remove, *btn_save;

	layout = elm_layout_add(fd->view->pager);
	elm_layout_file_set(layout, phoneui_theme, "phoneui/contacts/edit_field");
	/*Used for callbacks*/
	fd->edit_widget = content;
	elm_layout_content_set(layout, "main", content);

	btn_save = elm_button_add(fd->view->pager);

	elm_object_text_set(btn_save, D_("Save"));
	evas_object_smart_callback_add(btn_save, "clicked",
				       save_cb, fd);
	elm_layout_content_set(layout, "button_save", btn_save);
	evas_object_show(btn_save);

	btn_back = elm_button_add(fd->view->pager);
	elm_object_text_set(btn_back, D_("Back"));
	evas_object_smart_callback_add(btn_back, "clicked",
				       _field_edit_button_back_clicked_cb, fd);
	elm_layout_content_set(layout, "button_back", btn_back);
	evas_object_show(btn_back);

	btn_remove = elm_button_add(fd->view->pager);
	elm_object_text_set(btn_remove, D_("Remove"));
	evas_object_smart_callback_add(btn_remove, "clicked",
				       _field_edit_button_remove_clicked_cb, fd);
	elm_layout_content_set(layout, "button_remove", btn_remove);
	evas_object_show(btn_remove);

	elm_pager_content_push(fd->view->pager, layout);
}

static void
_field_edit_fileselector_save_cb(void *data, Evas_Object *obj, void *event_info)
{
	(void) obj;
	(void) event_info;
	const char *selected;
	struct ContactFieldData *fd = data;
	elm_pager_content_pop(fd->view->pager);
	selected = elm_fileselector_selected_get(fd->edit_widget);
	if (selected) {
		elm_entry_entry_set(fd->value_entry, selected);
		_change_value(fd, selected);
	}
}

/* genlist callbacks */

static void
_field_edit_clicked(void *data, Evas_Object *obj, void *event_info)
{
	(void) obj;
	(void) event_info;
	struct ContactFieldData *fd = data;
	if (!strcmp(fd->type, "photo")) {
		Evas_Object *content;
		content = _start_file_selector(fd->view->pager, getenv("HOME"));
		_field_edit_add_edit_page(fd, content, _field_edit_fileselector_save_cb);
	}
	else {
		edje_object_signal_emit((Evas_Object *) elm_genlist_item_object_get(fd->item), "start_edit", "elm");
		elm_entry_editable_set(fd->value_entry, EINA_TRUE);
		elm_object_focus_set(fd->value_entry, EINA_FALSE);
	}
	fd->edit_on = 1;
	_set_modify(fd->view, 1);
}


static Evas_Object *
gl_field_content_get(void *_data, Evas_Object * obj, const char *part)
{
	g_debug("gl_field_content_get (part=%s)", part);
	struct ContactFieldData *fd = (struct ContactFieldData *) _data;
	if (strcmp(part, "elm.swallow.field_button") == 0) {
		Evas_Object *btn = elm_button_add(obj);
		elm_object_text_set(btn, fd->name);
		evas_object_smart_callback_add(btn, "clicked",
					       _field_clicked, fd);
		fd->field_button = btn;
		return btn;
	}
	else if (strcmp(part, "elm.swallow.value_entry") == 0) {
		Evas_Object *entry = elm_entry_add(obj);
		elm_entry_entry_set(entry, fd->value);
		evas_object_size_hint_align_set(entry, 0.5,
						0.5);
		evas_object_size_hint_align_set(entry, 0.5,
						0.5);
		fd->value_entry = entry;
		elm_entry_editable_set(fd->value_entry, EINA_FALSE);
		return entry;
	}
	else if (strcmp(part, "elm.swallow.button_delfield") == 0) {
		Evas_Object *ico = elm_icon_add(obj);
		elm_icon_standard_set(ico, "delete");
		evas_object_smart_callback_add(ico, "clicked",
					       _field_remove_clicked, fd);
		return ico;
	}
	else if (strcmp(part, "elm.swallow.button_actions") == 0) {
		Evas_Object *ico = elm_icon_add(obj);
		elm_icon_standard_set(ico, "edit");
     		evas_object_smart_callback_add(ico, "clicked",
					      _field_edit_clicked, fd);
		fd->slide_buttons = ico;
		return ico;
	}
	return NULL;
}

static void
gl_field_del(void *_data, Evas_Object * obj)
{
	(void) obj;
	g_debug("gl_field_del");
	struct ContactFieldData *fd = (struct ContactFieldData *) _data;
	if (fd) {
		if (fd->name)
			free(fd->name);
		if (fd->type)
			free(fd->type);
		if (fd->value)
			free(fd->value);
		free(fd);
	}
	g_debug("gl_field_del DONE");
}

static void
_load_name(struct ContactViewData *view)
{
	char *s = NULL;
	g_debug("Loading name");
	if (view->properties) {
 		s = phoneui_utils_contact_display_name_get(view->properties);
	}
	if (s) {
		g_debug("Found name '%s'", s);
		ui_utils_view_text_set(&view->parent, "name", s);
		elm_win_title_set(ui_utils_view_window_get(VIEW_PTR(*view)), s);
		free(s);
	}
	else {
		g_debug("No name found");
		ui_utils_view_text_set(&view->parent, "name", "");
	}
}

static void
_load_number(struct ContactViewData *view)
{
	char *s = NULL;

	g_debug("Loading number");

	if (view->properties) {
		s = phoneui_utils_contact_display_phone_get(view->properties);
	}
	if (s) {
		g_debug("Found number '%s'", s);
		elm_object_disabled_set(view->btn_call, EINA_FALSE);
		elm_object_disabled_set(view->btn_sms, EINA_FALSE);
		ui_utils_view_text_set(VIEW_PTR(*view), "number", s);
		free(s);
	}
	else {
		g_debug("No number found");
		elm_object_disabled_set(view->btn_call, EINA_TRUE);
		elm_object_disabled_set(view->btn_sms, EINA_TRUE);
		ui_utils_view_text_set(VIEW_PTR(*view), "number", "");
	}
}

static void
_load_photo(struct ContactViewData *view)
{
	const char *s;
	GVariant *tmp = NULL;

	g_debug("Loading photo");
	if (view->properties) {
		tmp = g_hash_table_lookup(view->properties, "Photo");
	}
	if (tmp) {
		s = g_variant_get_string(tmp, NULL);
		g_debug("Found photo %s", s);
	}
	else {
		g_debug("No photo found");
		s = CONTACT_DEFAULT_PHOTO;
	}
	elm_icon_file_set(view->photo, s, NULL);
}

static void
_load_fields(struct ContactViewData *view)
{
	GHashTableIter iter;
	gpointer _key, _val;
	int isnew = 0;

	g_debug("Loading field list");

	/* mark all fields as new when the contact is new */
	if (!*view->path)
		isnew = 1;

	common_utils_debug_dump_hashtable(view->properties);

	elm_gen_clear(view->fields);
	if (view->properties) {
		g_hash_table_iter_init(&iter, view->properties);
		while (g_hash_table_iter_next(&iter, &_key, &_val)) {
			const char *key = (const char *)_key;
			GVariant *val = (GVariant *) _val;
			if (!strcmp(key, "Path") || !strcmp(key, "EntryId"))
				continue;
			if (g_variant_is_of_type(val, G_VARIANT_TYPE_STRING_ARRAY)) {
				const gchar **vl = g_variant_get_strv(val, NULL);
				int i = 0;
				while (vl[i]) {
					g_debug("--- %s", vl[i]);
					_add_field(view, key, vl[i], isnew);
					if (isnew) {
						_update_changes_of_field(view, key, "", vl[i]);
					}
					i++;
				}
			}
			else if (g_variant_is_of_type(val, G_VARIANT_TYPE_STRING)) {
				_add_field(view, key, g_variant_get_string(val, NULL),
					   isnew);
				if (isnew) {
					_update_changes_of_field(view, key, "", g_variant_get_string(val, NULL));
				}
			}
			else {
				g_warning("Value is neither string nor boxed!");
			}
		}
	}
	g_debug("Adding fields done");
}

struct _add_field_pack {
	struct ContactViewData *view;
	const char *name;
	const char *value;
	int isnew;
};

static void
_add_field_type_cb(GError *error, char *type, gpointer data)
{
	struct _add_field_pack *pack = data;
	struct ContactFieldData *fd =
		malloc(sizeof(struct ContactFieldData));
	if (error || !type) {
		type = strdup("generic");
	}
	if (!fd) {
		g_critical("Failed allocating field data!");
		return;
	}
	fd->name = strdup(pack->name);
	fd->value = strdup(pack->value);
	fd->field_button = NULL;
	fd->value_entry = NULL;
	fd->slide_buttons = NULL;
	fd->view = pack->view;
	fd->isnew = pack->isnew;
	fd->type = strdup(type);
	fd->edit_on = 0;
	if (fd->isnew) {
		fd->item = elm_genlist_item_prepend(pack->view->fields, &itc, fd, NULL,
				ELM_GENLIST_ITEM_NONE, NULL, NULL);
		elm_genlist_item_bring_in(fd->item);
	}
	else {
		fd->item = elm_genlist_item_append(pack->view->fields, &itc, fd, NULL,
				ELM_GENLIST_ITEM_NONE, NULL, NULL);
	}
	free(data);
	/*FIXME: free type */

}

static void
_add_field(struct ContactViewData *view,
	   const char *name, const char *value, int isnew)
{
	struct _add_field_pack *pack;
	/*FIXME: check success*/
	pack = malloc(sizeof (*pack));
	pack->view = view;
	pack->name = name;
	pack->value = value;
	pack->isnew = isnew;
	g_debug("Adding field <%s> with value '%s' to list", name, value);
	phoneui_utils_contacts_field_type_get(name, _add_field_type_cb, pack);
}


static void
_delete_cb(struct View *view, Evas_Object * win, void *event_info)
{
	(void)win;
	(void)event_info;
	g_debug("_delete_cb");
	contact_view_deinit((struct ContactViewData *)view);
	free(view);
	g_debug("_delete_cb DONE");
}

static void
_destroy_cb(struct View *_view)
{
	struct ContactViewData *view = (struct ContactViewData *)_view;
	g_debug("_destroy_cb");
	phoneui_info_unregister_single_contact_changes(view->entryid,
						       _contact_changed_handler);
	if (view->properties)
		g_hash_table_destroy(view->properties);
	if (view->changes)
		g_hash_table_destroy(view->changes);
	g_hash_table_remove(contactviews, view->path);

	g_debug("_destroy_cb DONE");
}

static void
_contact_changed_handler(void *data, int entryid, enum PhoneuiInfoChangeType type)
{
	(void) entryid;
	struct ContactViewData *view;

	view = data;
	if (type == PHONEUI_INFO_CHANGE_DELETE) {
		contact_view_deinit(view);
	}
	else if (type == PHONEUI_INFO_CHANGE_UPDATE) {
		phoneui_utils_contact_get(view->path, _load_cb, view);
	}
}

static void
_field_unselected_cb(void *data, Evas_Object *obj, void *event_info)
{
	struct ContactFieldData *fd = (struct ContactFieldData *) elm_genlist_item_data_get(event_info);
	(void) obj;
	(void) data;
	if (!fd->edit_on) {
		return;
	}
	char *s = ui_utils_entry_utf8_get(fd->value_entry);
	_change_value(fd, s);
	free(s);
	elm_entry_editable_set(fd->value_entry, EINA_FALSE);
	fd->edit_on = 0;
}
