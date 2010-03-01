#include <glib.h>
#include <glib-object.h>
#include <Elementary.h>
#include <phoneui/phoneui.h>
#include <phoneui/phoneui-utils.h>
#include "common-utils.h"
#include "ui-utils.h"
#include "views.h"
#include "message-show-view.h"


static GHashTable *messageviews = NULL;

static void _close_clicked(void *_data, Evas_Object * obj, void *event_info);
static void _answer_clicked(void *_data, Evas_Object * obj, void *event_info);
static void _delete_clicked(void *_data, Evas_Object * obj, void *event_info);
static void _call_clicked(void *_data, Evas_Object * obj, void *event_info);
static void _new_contact_clicked(void *_data, Evas_Object * obj, void *event_info);
static void _hover_bt_1(void *_data, Evas_Object * obj, void *event_info);
static void _common_name_callback(GHashTable *contact, void *_data);
static void _delete_cb(struct View *view, Evas_Object * win, void *event_info);
static void _destroy_cb(struct View *_view);


int
message_show_view_init(char* path, GHashTable *properties)
{
	struct MessageShowViewData *view;
	Evas_Object *win, *ico;
	int ret;
	GValue *tmp;
	int status = 0;
	const char *direction = NULL;

	/* path MUST always be set! It will be freed by
	destroying the messageviews hashtable in here, thus must be a copy */
	if (!path) {
		g_warning("Trying to initialize a message view without path!");
		return 1;
	}

	g_debug("Initializing the contact view for '%s'", path);

	view = malloc(sizeof(struct MessageShowViewData));
	if (!view) {
		g_critical("Failed to allocate message view for '%s'", path);
		if (properties) {
			g_hash_table_unref(properties);
		}
		free(path);
		return 1;
	}

	ret = ui_utils_view_init(VIEW_PTR(*view), ELM_WIN_BASIC, D_("Message"),
				 NULL, NULL, _destroy_cb);
	if (ret) {
		g_critical("Failed to init message view for '%s'", path);
		if (properties) {
			g_hash_table_unref(properties);
		}
		free(view);
		free(path);
		return ret;
	}

	/* cache the views to open only one view per message */
	if (messageviews == NULL) {
		messageviews = g_hash_table_new_full(g_str_hash, g_str_equal,
						     free, NULL);
	}
	g_hash_table_insert(messageviews, path, view);

	view->path = path;
	view->number = NULL;
	view->name = NULL;
	view->photopath = NULL;

	elm_theme_extension_add(DEFAULT_THEME);

	win = ui_utils_view_window_get(VIEW_PTR(*view));
	ui_utils_view_delete_callback_set(VIEW_PTR(*view), _delete_cb);

	ui_utils_view_layout_set(VIEW_PTR(*view), DEFAULT_THEME,
				 "phoneui/messages/show");

// 	char *content = elm_entry_utf8_to_markup(data->content);

	GList *keys = g_hash_table_get_keys(properties);
	for (; keys; keys = keys->next) {
		tmp = g_hash_table_lookup(properties, keys->data);
		if (tmp) {
			if (G_VALUE_HOLDS_STRING(tmp)) {
				g_debug("--- %s: '%s'", (char *)keys->data,
					g_value_get_string(tmp));
			}
			else if (G_VALUE_HOLDS_INT(tmp)) {
				g_debug("--- %s: %d", (char *)keys->data,
					g_value_get_int(tmp));
			}
			else {
				g_debug("--- %s: No string and no int!",
					(char *)keys->data);
			}
		}
	}

	tmp = g_hash_table_lookup(properties, "Sender");
	if (!tmp) {
		tmp = g_hash_table_lookup(properties, "Recipient");
	}
	if (tmp) {
		view->number = strdup(g_value_get_string(tmp));
		g_debug("Found number %s - starting lookup", view->number);
		phoneui_utils_contact_lookup(view->number,
					     _common_name_callback,
					     common_utils_object_ref(view));
		ui_utils_view_text_set(VIEW_PTR(*view), "text_number",
				       view->number);
	}

	tmp = g_hash_table_lookup(properties, "Timestamp");
	if (tmp) {
		char *date = common_utils_timestamp_to_date(
					(long)g_value_get_int(tmp));
		if (date) {
			g_debug("Found date %s", date);
			ui_utils_view_text_set(VIEW_PTR(*view), "text_date", date);
			free(date);
		}
	}

        view->photo = elm_icon_add(win);
	evas_object_size_hint_aspect_set(view->photo,
					 EVAS_ASPECT_CONTROL_VERTICAL, 1, 1);
	elm_icon_file_set(view->photo, DEFAULT_THEME, "icon/contact");
	ui_utils_view_swallow(VIEW_PTR(*view), "photo", view->photo);
	evas_object_show(view->photo);

	ico = elm_icon_add(win);
	tmp = (GValue *)g_hash_table_lookup(properties, "MessageRead");
	if (tmp) {
		status = g_value_get_int(tmp);
	}
	tmp = (GValue *)g_hash_table_lookup(properties, "Direction");
	if (tmp) {
		direction = g_value_get_string(tmp);
		if (strcmp(direction, "in") == 0) {
			g_debug("Setting status icon for an incoming message");
			elm_icon_file_set(ico, DEFAULT_THEME,
					  "icon/phonelog-incoming");
		}
		else {
			g_debug("Setting status icon for a sent message");
			elm_icon_file_set(ico, DEFAULT_THEME,
					  "icon/phonelog-outgoing");
		}
	}
	ui_utils_view_swallow(VIEW_PTR(*view), "icon_status", ico);
	evas_object_show(ico);
/*	if (status && direction) {
		char *str = malloc(strlen(status) + strlen(direction) + 4);
		if (str) {
			sprintf(str, "%s (%s)", status, direction);
			ui_utils_view_text_set(VIEW_PTR(*view), "text_status", str);
			free(str);
		}
	}
*/
	const char *content = NULL;
	tmp = g_hash_table_lookup(properties, "Content");
	if (tmp) {
		content = g_value_get_string(tmp);
// 		content = elm_entry_utf8_to_markup(data->content);

	}

	view->sc_content = elm_scroller_add(win);
	elm_scroller_bounce_set(view->sc_content, EINA_FALSE, EINA_FALSE);

	view->content = elm_anchorblock_add(win);
	elm_anchorblock_hover_style_set(view->content, "popout");
	elm_anchorblock_hover_parent_set(view->content, win);
	evas_object_size_hint_weight_set(view->content, EVAS_HINT_EXPAND,
					 EVAS_HINT_EXPAND);
	if (content) {
		elm_anchorblock_text_set(view->content, content);
	}
	elm_scroller_content_set(view->sc_content, view->content);
	evas_object_show(view->content);
	ui_utils_view_swallow(VIEW_PTR(*view), "text_content", view->sc_content);
	evas_object_show(view->sc_content);


	view->bt1 = elm_button_add(win);
	elm_button_label_set(view->bt1, D_("Close"));
	evas_object_smart_callback_add(view->bt1, "clicked",
				       _close_clicked, view);
	ui_utils_view_swallow(VIEW_PTR(*view), "button_close", view->bt1);
	evas_object_show(view->bt1);

	// Options button with hover
	view->hv = elm_hover_add(win);

	view->bt2 = elm_button_add(win);
	elm_button_label_set(view->bt2, D_("Options"));
	evas_object_smart_callback_add(view->bt2, "clicked", _hover_bt_1,
				       view->hv);
	ui_utils_view_swallow(VIEW_PTR(*view), "button_options", view->bt2);
	evas_object_show(view->bt2);

	elm_hover_parent_set(view->hv, win);
	elm_hover_target_set(view->hv, view->bt2);

	view->bx = elm_box_add(win);
	elm_box_horizontal_set(view->bx, 0);
	elm_box_homogenous_set(view->bx, 1);
	evas_object_show(view->bx);

	view->hbt1 = elm_button_add(win);
	elm_button_label_set(view->hbt1, D_("Delete"));
	evas_object_size_hint_min_set(view->hbt1, 140, 80);
	evas_object_smart_callback_add(view->hbt1, "clicked",
				       _delete_clicked, view);
	evas_object_show(view->hbt1);
	elm_box_pack_end(view->bx, view->hbt1);

	view->hbt2 = elm_button_add(win);
	elm_button_label_set(view->hbt2, D_("Call"));
	evas_object_size_hint_min_set(view->hbt2, 140, 80);
	evas_object_smart_callback_add(view->hbt2, "clicked",
				       _call_clicked, view);
	evas_object_show(view->hbt2);
	elm_box_pack_end(view->bx, view->hbt2);

	view->hbt3 = elm_button_add(win);
	elm_button_label_set(view->hbt3, D_("Add Contact"));
	evas_object_size_hint_min_set(view->hbt3, 140, 80);
	evas_object_smart_callback_add(view->hbt3, "clicked",
				       _new_contact_clicked,
				       view);
	evas_object_show(view->hbt3);
	elm_box_pack_end(view->bx, view->hbt3);

	elm_hover_content_set(view->hv, "top", view->bx);


	view->bt3 = elm_button_add(win);
	elm_button_label_set(view->bt3, D_("Answer"));
	evas_object_smart_callback_add(view->bt3, "clicked",
				       _answer_clicked, view);
	ui_utils_view_swallow(VIEW_PTR(*view), "button_answer", view->bt3);
	evas_object_show(view->bt3);

	phoneui_utils_message_set_read_status(view->path, 1, NULL, NULL);

	g_hash_table_destroy(properties);

	return 0;
}

int
message_show_view_is_init(const char* path)
{
	struct MessageShowViewData *view;

	if (messageviews == NULL) {
		return 0;
	}
	view = (struct MessageShowViewData *)
			g_hash_table_lookup(messageviews, path);
	if (view && ui_utils_view_is_init(VIEW_PTR(*view))) {
		return 1;
	}
	return 0;
}

void
message_show_view_deinit(struct MessageShowViewData *view)
{
	if (view) {
		g_debug("Deiniting view for message");
		ui_utils_view_deinit(VIEW_PTR(*view));
	}
	else {
		g_warning("Deiniting a message view without view?");
	}
}

void
message_show_view_show(const char* path)
{
	struct MessageShowViewData *view;

	g_debug("looking up message view for '%s'", path);

	if (messageviews == NULL) {
		g_debug("No message views loaded yet");
		return;
	}
	view = (struct MessageShowViewData *)
			g_hash_table_lookup(messageviews, path);
	if (view) {
		ui_utils_view_show(VIEW_PTR(*view));
	}
	else {
		g_warning("Could not find view for message '%s'", path);
	}
	g_debug("message view show done");

}

void
message_show_view_hide(const char* path)
{
	(void) path;
}

/* --- evas callbacks ------------------------------------------------------- */

static void
_close_clicked(void *_data, Evas_Object * obj,
				void *event_info)
{
	(void) obj;
	(void) event_info;
	struct MessageShowViewData *view = (struct MessageShowViewData *) _data;
	message_show_view_deinit(view);
	free(view);
}

static void
_answer_clicked(void *_data, Evas_Object * obj,
				 void *event_info)
{
	(void) obj;
	(void) event_info;
	(void) _data;
// 	struct MessageShowViewData *data = (struct MessageShowViewData *) _data;

	g_debug("message_show_view_answer_clicked()");

	GHashTable *options = g_hash_table_new(g_str_hash, g_str_equal);
// 	g_hash_table_insert(options, "Name", common_utils_new_gvalue_string(data->name));
// 	g_hash_table_insert(options, "Phone", common_utils_new_gvalue_string(data->number));

	phoneui_messages_message_new(options);
	//g_hash_table_destroy(options);
}

static void
_call_clicked(void *_data, Evas_Object * obj, void *event_info)
{
	(void) obj;
	(void) event_info;
	struct MessageShowViewData *view = (struct MessageShowViewData *) _data;
	char *number = view->number;
	g_debug("_call_clicked()");

	phoneui_utils_dial(number, NULL, NULL);
}

static void
_delete_result_cb(GError *error, gpointer data)
{
	struct MessageShowViewData *view = (struct MessageShowViewData *)data;
	if (error) {
		ui_utils_dialog(VIEW_PTR(view),
				D_("Deleting the message failed!"), DIALOG_OK,
				NULL, NULL);
		g_warning("Deleting the message failed: %s", error->message);
		return;
	}
	message_show_view_deinit(view);
}

static void
_delete_confirm_cb(int result, void *data)
{
	struct MessageShowViewData *view = (struct MessageShowViewData *)data;
	if (result == DIALOG_YES && view->path) {
		phoneui_utils_message_delete(view->path, _delete_result_cb,
					     view);
	}
}

static void
_delete_clicked(void *_data, Evas_Object * obj, void *event_info)
{
	(void) obj;
	(void) event_info;
	struct MessageShowViewData *view = (struct MessageShowViewData *) _data;

	g_debug("_delete_clicked()");

	evas_object_hide(view->hv);
	ui_utils_dialog(VIEW_PTR(*view),
				D_("Really delete this message?"),
				DIALOG_YES|DIALOG_NO,
				_delete_confirm_cb, view);

}

static void
_hover_bt_1(void *_data, Evas_Object * obj, void *event_info)
{
	(void) obj;
	(void) event_info;
	Evas_Object *hv = (Evas_Object *) _data;
	evas_object_show(hv);
}



/* callbacks */


static void
_common_name_callback(GHashTable *contact, void *_data)
{
	struct MessageShowViewData *view = (struct MessageShowViewData *) _data;
	char *tmp;
	GValue *gval_tmp;

	if (!contact)
		return;

	if (!ui_utils_view_is_init(VIEW_PTR(*view))) {
		return;
	}

	tmp = phoneui_utils_contact_display_name_get(contact);
	if (tmp) {
		ui_utils_view_text_set(VIEW_PTR(*view), "text_number", tmp);
		free(tmp);
	}
	gval_tmp = g_hash_table_lookup(contact, "Photo");
	if (gval_tmp) {
		elm_icon_file_set(view->photo,
				  g_value_get_string(gval_tmp), NULL);
	}
}


static void
_new_contact_clicked(void *_data, Evas_Object * obj, void *event_info)
{
	(void) obj;
	(void) event_info;
	struct MessageShowViewData *view;
	view = (struct MessageShowViewData *)_data;
	evas_object_hide(view->hv);
	GHashTable *options = g_hash_table_new_full(g_str_hash, g_str_equal,
					       NULL, common_utils_gvalue_free);
	g_hash_table_insert(options, "Phone",
			common_utils_new_gvalue_string(view->number));

	phoneui_contacts_contact_new(options);
}



static void
_delete_cb(struct View *view, Evas_Object * win, void *event_info)
{
	(void)win;
	(void)event_info;
	g_debug("_delete_cb");
	message_show_view_deinit((struct MessageShowViewData *)view);
	free(view);
	g_debug("_delete_cb DONE");
}

static void
_destroy_cb(struct View *_view)
{
	struct MessageShowViewData *view = (struct MessageShowViewData *)_view;
	g_debug("_destroy_cb");
	g_hash_table_remove(messageviews, view->path);
	g_debug("_destroy_cb DONE");
}

