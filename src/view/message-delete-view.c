#include "views.h"
#include <phoneui/phoneui-utils.h>


struct MessageDeleteViewData {
	struct Window *win;
	char *path;
	void (*callback) ();
	void *callback_data;
	Evas_Object *bt_yes, *bt_no;
};


void
message_delete_yes_clicked(void *userdata, Evas_Object * obj, void *event_info);
void
message_delete_no_clicked(void *userdata, Evas_Object * obj, void *event_info);
static void
delete_callback(GError * error, gpointer _data);

static void
frame_delete_show(void *_data);
static void
frame_delete_hide(void *_data);
static void
frame_delete_no_clicked(void *_data, Evas_Object * obj, void *event_info);
static void
frame_delete_yes_clicked(void *_data, Evas_Object * obj, void *event_info);

static void
frame_deleting_show(void *data);



/* --- message delete view -------------------------------------------------- */

void *
message_delete_view_show(struct Window *win, void *_options)
{
	GHashTable *options = (GHashTable *) _options;

	g_debug("message_delete_view_show()");

	struct MessageDeleteViewData *data =
		g_slice_alloc0(sizeof(struct MessageDeleteViewData));
	data->win = win;

	if (options == NULL)
		g_error("At least option[path] must be set.");
	else {
		data->path = g_hash_table_lookup(options, "path");
		data->callback =
			g_hash_table_lookup(options, "delete_callback");
		data->callback_data =
			g_hash_table_lookup(options, "delete_callback_data");

		g_debug("Delete view with message path = %s", data->path);
	}

	window_frame_show(win, data, frame_delete_show, frame_delete_hide);
	window_show(win);

	return data;
}

void
message_delete_view_hide(void *data)
{
	g_debug("message_delete_view_hide()");

	g_slice_free(struct MessageDeleteViewData, data);
}

static void
delete_callback(GError * error, gpointer _data)
{
	struct MessageDeleteViewData *data =
		(struct MessageDeleteViewData *) _data;

	if (data->callback)
		data->callback(data->callback_data);

	window_destroy(data->win, NULL);
}



/* --- frame "delete" ------------------------------------------------------- */

static void
frame_delete_show(void *_data)
{
	struct MessageDeleteViewData *data =
		(struct MessageDeleteViewData *) _data;
	struct Window *win = data->win;

	g_debug("frame_delete_show()");

	window_layout_set(win, DEFAULT_THEME, "phoneui/messages/delete");

	window_text_set(win, "info", D_("Do you really want to delete it?"));

	data->bt_yes = elm_button_add(window_evas_object_get(win));
	elm_button_label_set(data->bt_yes, D_("Yes"));
	evas_object_smart_callback_add(data->bt_yes, "clicked",
				       frame_delete_yes_clicked, data);
	window_swallow(win, "button_yes", data->bt_yes);
	evas_object_show(data->bt_yes);

	data->bt_no = elm_button_add(window_evas_object_get(win));
	elm_button_label_set(data->bt_no, D_("No"));
	evas_object_smart_callback_add(data->bt_no, "clicked",
				       frame_delete_no_clicked, data);
	window_swallow(win, "button_no", data->bt_no);
	evas_object_show(data->bt_no);
}

static void
frame_delete_hide(void *_data)
{
	struct MessageDeleteViewData *data =
		(struct MessageDeleteViewData *) _data;

	g_debug("frame_delete_hide()");

	evas_object_del(data->bt_yes);
	evas_object_del(data->bt_no);
}

static void
frame_delete_no_clicked(void *_data, Evas_Object * obj, void *event_info)
{
	struct MessageDeleteViewData *data =
		(struct MessageDeleteViewData *) _data;

	g_debug("frame_delete_no_clicked()");

	window_destroy(data->win, NULL);
}

static void
frame_delete_yes_clicked(void *_data, Evas_Object * obj, void *event_info)
{
	struct MessageDeleteViewData *data =
		(struct MessageDeleteViewData *) _data;

	g_debug("frame_delete_yes_clicked()");

	window_frame_show(data->win, data, frame_deleting_show, NULL);

	phoneui_utils_message_delete(data->path, delete_callback, data);
}



/* --- frame "deleting" ----------------------------------------------------- */

static void
frame_deleting_show(void *_data)
{
	struct MessageDeleteViewData *data =
		(struct MessageDeleteViewData *) _data;
	struct Window *win = data->win;

	g_debug("frame_deleting_show()");

	window_layout_set(win, DEFAULT_THEME, "phoneui/messages/deleting");
	window_text_set(win, "text", D_("Deleting.."));
}

