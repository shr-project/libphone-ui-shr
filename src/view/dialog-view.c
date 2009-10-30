
#include "views.h"
#include <phoneui/phoneui-utils.h>

struct DialogViewData {
	struct Window *win;
	Evas_Object *bt_close;
	int type;
};



static void
  dialog_view_close_clicked(void *_data, Evas_Object * obj, void *event_info);


void *
dialog_view_show(struct Window *win, void *_options)
{
	GHashTable *options = (GHashTable *) _options;

	g_debug("dialog_view_show()");

	struct DialogViewData *data =
		g_slice_alloc0(sizeof(struct DialogViewData));
	data->win = win;
	data->type = GPOINTER_TO_INT(g_hash_table_lookup(options, "type"));

	// Check if type was provided
	gboolean type_exists =
		g_hash_table_lookup_extended(options, "type", NULL, NULL);
	assert(type_exists == TRUE);

	window_layout_set(win, DEFAULT_THEME, "phoneui/notification/dialog");
	if (data->type == PHONEGUI_DIALOG_MESSAGE_STORAGE_FULL)
		window_text_set(win, "content",
				D_
				("Your storage is full. Please delete some messages or you are not going to receive messages anymore!"));
	else if (data->type == PHONEGUI_DIALOG_SIM_NOT_PRESENT)
		window_text_set(win, "content",
				D_
				("GSM is not available, because no SIM card is present."));
	else
		window_text_set(win, "content", D_("Unknown message."));

	data->bt_close = elm_button_add(win->win);
	elm_button_label_set(data->bt_close, D_("Close"));
	evas_object_smart_callback_add(data->bt_close, "clicked",
				       dialog_view_close_clicked, data);
	window_swallow(win, "button_close", data->bt_close);
	evas_object_show(data->bt_close);


	window_show(win);
	return data;
}

void
dialog_view_hide(void *_data)
{
	struct DialogViewData *data = (struct DialogViewData *) _data;
	evas_object_del(data->bt_close);
}

static void
dialog_view_close_clicked(void *_data, Evas_Object * obj, void *event_info)
{
	struct DialogViewData *data = (struct DialogViewData *) _data;
	window_destroy(data->win, NULL);
}
