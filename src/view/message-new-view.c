
#include <phoneui/phoneui-utils.h>
#include <phone-utils.h>

#include "ui-utils.h"
#include "util/common-utils.h"
#include "message-new-view.h"
#include "views.h"


enum MessageNewModes {
	MODE_CONTENT,
	MODE_RECIPIENT,
	MODE_RECIPIENT_NUMBER,
	MODE_RECIPIENT_CONTACT,
	MODE_CLOSE
};


static Elm_Genlist_Item_Class itc;


static void _init_content_page(struct MessageNewViewData *view);
static void _init_recipient_page(struct MessageNewViewData *view);
static void _left_button_clicked(void *data, Evas_Object *obj, void *event_info);
static void _right_button_clicked(void *data, Evas_Object *obj, void *event_info);

static void _content_changed(void *_data, Evas_Object * obj, void *event_info);
static void _contact_add_clicked(void *_data, Evas_Object * obj, void *event_info);
static void _number_add_clicked(void *_data, Evas_Object * obj, void *event_info);
static void _recipient_delete_clicked(void *_data, Evas_Object * obj, void *event_info);
static void _process_recipient(gpointer _properties, gpointer _data);
static char *gl_label_get(const void *data, Evas_Object * obj, const char *part);
static Evas_Object *gl_icon_get(const void *data, Evas_Object * obj, const char *part);

static void _destroy_cb(struct View *view);

struct MessageNewViewData *
message_new_view_init(GHashTable *options)
{
	struct MessageNewViewData *view;
	int ret;
	Evas_Object *win;

	view = malloc(sizeof(struct MessageNewViewData));
	if (!view) {
		g_critical("Failed to allocate new message view");
		if (options) {
			g_hash_table_unref(options);
		}
		return NULL;
	}

	ret = ui_utils_view_init(VIEW_PTR(*view), ELM_WIN_BASIC,
				  D_("New Message"), NULL, NULL, _destroy_cb);
	if (ret) {
		g_critical("Failed to init new message view");
		if (options) {
			g_hash_table_unref(options);
		}
		free(view);
		return NULL;
	}

	view->mode = MODE_CONTENT;
	view->content = NULL;
	view->recipients = g_ptr_array_new();
	view->messages_sent = 0;
	view->cdata = NULL;
	if (options) {
		g_ptr_array_add(view->recipients, options);
	}

	elm_theme_extension_add(DEFAULT_THEME);
	ui_utils_view_layout_set(VIEW_PTR(*view), DEFAULT_THEME,
				 "phoneui/messages/new");
	win = ui_utils_view_window_get(VIEW_PTR(*view));

/*	view->pager = elm_pager_add(win);
	ui_utils_view_swallow(VIEW_PTR(*view), "pager", view->pager);
	evas_object_show(view->pager);*/

	_init_content_page(view);
	_init_recipient_page(view);

	view->left_button = elm_button_add(win);
	elm_button_label_set(view->left_button, D_("Close"));
	evas_object_smart_callback_add(view->left_button, "clicked",
				       _left_button_clicked, view);
	ui_utils_view_swallow(VIEW_PTR(*view), "left_button", view->left_button);
	evas_object_show(view->left_button);

	view->right_button = elm_button_add(win);
	elm_button_label_set(view->right_button, D_("Continue"));
	evas_object_smart_callback_add(view->right_button, "clicked",
				       _right_button_clicked, view);
	ui_utils_view_swallow(VIEW_PTR(*view), "right_button", view->right_button);
	evas_object_show(view->right_button);

	return view;
}


void
message_new_view_deinit(struct MessageNewViewData *view)
{
	if (view) {
		ui_utils_view_deinit(VIEW_PTR(*view));
	}
	else {
		g_warning("Deiniting a new message view without view?");
	}
}

void
message_new_view_show(struct MessageNewViewData *view)
{
	if (view) {
		ui_utils_view_show(VIEW_PTR(*view));
	}
}


static char *
gl_label_get(const void *data, Evas_Object * obj, const char *part)
{
	(void) obj;
	const char *label = NULL;
	GHashTable *parameters = (GHashTable *) data;
	g_debug("looking for %s", part);

	/* Memory leak: */
	if (!strcmp(part, "elm.text")) {
		label = phoneui_utils_contact_display_name_get(parameters);
		if (!label) {
			return strdup("Number");
		}
	}
	else if (!strcmp(part, "elm.text.sub")) {
		label = phoneui_utils_contact_display_phone_get(parameters);
	}
	/*FIXME: leaks? */
	return strdup(label);
}

static Evas_Object *
gl_icon_get(const void *data, Evas_Object * obj, const char *part)
{
	g_debug("gl_icon_get: %s", part);
	GHashTable *parameters = (GHashTable *) data;
	if (!strcmp(part, "elm.swallow.icon")) {
		const char *photo_file;
		GValue *tmp = g_hash_table_lookup(parameters, "Photo");
		photo_file = (tmp) ? g_value_get_string(tmp) : CONTACT_DEFAULT_PHOTO;
		Evas_Object *photo = elm_icon_add(obj);
		elm_icon_file_set(photo, photo_file, NULL);
		evas_object_size_hint_aspect_set(photo,
						 EVAS_ASPECT_CONTROL_VERTICAL,
						 1, 1);
		return (photo);
	}
	return (NULL);
}




//static void message_send_callback(GError *error, int transaction_index, struct MessageNewViewData *data);

static void
_init_content_page(struct MessageNewViewData *view)
{
	Evas_Object *win;

	win = ui_utils_view_window_get(VIEW_PTR(*view));

	view->box_content = elm_box_add(win);
	elm_win_resize_object_add(win, view->box_content);

	view->layout_content = elm_layout_add(view->box_content);
	elm_win_resize_object_add(win, view->layout_content);
	elm_layout_file_set(view->layout_content, DEFAULT_THEME,
			    "phoneui/messages/new/content");
	evas_object_show(view->layout_content);

	view->sc = elm_scroller_add(win);
	view->entry = elm_entry_add(win);
	evas_object_size_hint_weight_set(view->entry,
			EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_smart_callback_add(view->entry, "changed",
				       _content_changed, view);
	if (view->content != NULL)
		elm_entry_entry_set(view->entry, view->content);
	elm_scroller_content_set(view->sc, view->entry);
	evas_object_show(view->entry);

	elm_layout_content_set(view->layout_content, "entry", view->sc);
	evas_object_show(view->sc);

	elm_object_focus(view->entry);

	evas_object_show(view->box_content);

// 	elm_pager_content_push(view->pager, view->box_content);
}

static void
_init_recipient_page(struct MessageNewViewData *view)
{
	Evas_Object *win, *obj;

	win = ui_utils_view_window_get(VIEW_PTR(*view));

        view->box_recipients = elm_box_add(win);
	elm_win_resize_object_add(win, view->box_recipients);
	view->layout_recipients = elm_layout_add(view->box_recipients);
	elm_win_resize_object_add(win, view->layout_recipients);
	elm_layout_file_set(view->layout_recipients, DEFAULT_THEME,
			    "phoneui/messages/new/recipients");
	evas_object_show(view->layout_recipients);

	edje_object_part_text_set(elm_layout_edje_get(view->layout_recipients),
				  "title", D_("Define Recipients"));

	obj = elm_button_add(win);
	elm_button_label_set(obj, D_("Add Contact"));
	evas_object_smart_callback_add(obj, "clicked",
				       _contact_add_clicked, view);
	elm_layout_content_set(view->layout_recipients, "button_contact_add", obj);
	evas_object_show(obj);

	obj = elm_button_add(win);
	elm_button_label_set(obj, D_("Add Number"));
	evas_object_smart_callback_add(obj, "clicked",
				       _number_add_clicked, view);
	elm_layout_content_set(view->layout_recipients, "button_number_add", obj);
	evas_object_show(obj);

	obj = elm_button_add(win);
	elm_button_label_set(obj, D_("Remove"));
	evas_object_smart_callback_add(obj, "clicked",
				       _recipient_delete_clicked, view);
	elm_layout_content_set(view->layout_recipients, "button_delete", obj);
	evas_object_show(obj);

	view->list_recipients = elm_genlist_add(win);
	elm_genlist_horizontal_mode_set(view->list_recipients, ELM_LIST_LIMIT);
	evas_object_size_hint_align_set(view->list_recipients, 0.0, 0.0);
	elm_object_scale_set(view->list_recipients, 1.0);
	elm_layout_content_set(view->layout_recipients, "list", view->list_recipients);
	itc.item_style = "contact";
	itc.func.label_get = gl_label_get;
	itc.func.icon_get = gl_icon_get;
	itc.func.state_get = NULL;
	itc.func.del = NULL;
	evas_object_show(view->list_recipients);

	g_ptr_array_foreach(view->recipients, _process_recipient, view);

// 	elm_pager_content_push(view->pager, view->box_recipients);
}

static void
_left_button_clicked(void *data, Evas_Object *obj, void *event_info)
{
	(void) obj;
	(void) event_info;
	struct MessageNewViewData *view = (struct MessageNewViewData *)data;
	switch (view->mode) {
	case MODE_CONTENT:
		// TODO: ask for confirmation
		message_new_view_deinit(view);
		break;
	case MODE_RECIPIENT:
		view->mode = MODE_CONTENT;
/*		elm_pager_content_promote(view->pager,
					  view->box_content);*/
		evas_object_hide(view->box_recipients);
		evas_object_show(view->box_content);
		elm_button_label_set(view->left_button, D_("Close"));
		elm_button_label_set(view->right_button, D_("Continue"));
		break;
	}
}

static void
_right_button_clicked(void *data, Evas_Object *obj, void *event_info)
{
	(void) obj;
	(void) event_info;
	struct MessageNewViewData *view = (struct MessageNewViewData *)data;
	switch (view->mode) {
	case MODE_CONTENT:
		view->mode = MODE_RECIPIENT;
/*		elm_pager_content_promote(view->pager,
						view->box_recipients);*/
		evas_object_hide(view->box_recipients);
		evas_object_show(view->box_content);
		elm_button_label_set(view->left_button, D_("Back"));
		elm_button_label_set(view->right_button, D_("Send"));
		break;
	case MODE_RECIPIENT:
		if (view->recipients->len) {
			// TODO: show inwin for progress via callback
			phoneui_utils_sms_send(view->content, view->recipients, NULL, NULL);
		}
		break;
	}
}

static void
_content_changed(void *_data, Evas_Object * obj, void *event_info)
{
	(void) event_info;
	struct MessageNewViewData *view = (struct MessageNewViewData *) _data;
	char *content;
	int limit;		/* the limit of the sms */
	int len;		/* the number of characters in the sms */
	char text[64];

	/*FIXME: consider changing to an iterative way by using get_size (emulating what's
	 * being done in phone_utils) as calculating for all the string on every keystroke is a bit sluggish. */
	content = elm_entry_markup_to_utf8(elm_entry_entry_get(obj));
	/* if the entry is still empty elm_entry_markup_to_utf8 will return
	 * NULL - which makes g_strstrip segfault :|
	 * and we don't have to do all the fancy calculation
	 * if it is empty */
	if (!content) {
		sprintf(text, D_("%d characters left [%d]"),
				PHONE_UTILS_GSM_SMS_TEXT_LIMIT,
				PHONE_UTILS_GSM_SMS_TEXT_LIMIT);
		ui_utils_view_text_set(VIEW_PTR(*view), "characters_left", text);
		return;
	}

	len = phone_utils_gsm_sms_strlen(content);

	/* if it includes chars that can't be represented
	 * with 7bit encoding, this sms will be sent as ucs-2 treat
	 * it this way! */
	if (phone_utils_gsm_is_ucs(content)) {
		limit = PHONE_UTILS_GSM_SMS_UCS_LIMIT;	/* ucs-2 number of chars limit */
		if (len > limit) {
			limit = PHONE_UTILS_GSM_SMS_UCS_SPLIT_LIMIT;
		}
	}
	else {
		limit = PHONE_UTILS_GSM_SMS_TEXT_LIMIT;	/* regular number of chars limit */
		if (len > limit) {
			limit = PHONE_UTILS_GSM_SMS_TEXT_SPLIT_LIMIT;
		}
	}


	int left = limit - (len % limit);
	if (left == limit && (len / limit) + 1 > 1) {
		left = 0;
	}
	/*FIXME: BAD BAD BAD! will cause an overflow when using a long translation!!! */
	sprintf(text, D_("%d characters left [%d]"), left, (len / limit) + 1);
	ui_utils_view_text_set(VIEW_PTR(*view), "characters_left", text);
	if (view->content) {
		free(view->content);
	}
	view->content = content;
}

static void
_contact_add_clicked(void *_data, Evas_Object * obj, void *event_info)
{
	(void) obj;
	(void) event_info;
	struct MessageNewViewData *view = (struct MessageNewViewData *) _data;

	view->mode = MODE_RECIPIENT_CONTACT;
	// TODO !!!
}

static void
_number_add_clicked(void *_data, Evas_Object * obj, void *event_info)
{
	(void) obj;
	(void) event_info;
	struct MessageNewViewData *data = (struct MessageNewViewData *) _data;

	data->mode = MODE_RECIPIENT_NUMBER;
	// TODO !!!
}

static void
_recipient_delete_clicked(void *_data, Evas_Object * obj, void *event_info)
{
	(void) obj;
	(void) event_info;
	struct MessageNewViewData *data = (struct MessageNewViewData *) _data;

	Elm_Genlist_Item *it =
			elm_genlist_selected_item_get(data->list_recipients);
	if (it) {
		GHashTable *parameters = (GHashTable *) elm_genlist_item_data_get(it);
		g_ptr_array_remove(data->recipients, parameters);
		elm_genlist_item_del(it);
	}
}

static void
_process_recipient(gpointer _properties, gpointer _data)
{
	GHashTable *properties = (GHashTable *) _properties;
	//g_debug("adding recipient %s", g_hash_table_lookup(properties, "name"));
	struct MessageNewViewData *data = (struct MessageNewViewData *) _data;

	elm_genlist_item_append(data->list_recipients, &itc, properties,
		NULL, ELM_GENLIST_ITEM_NONE, NULL, NULL);
}



/* --- frame "contact_add" -------------------------------------------------- */
#if 0
static void
frame_contact_add_show(void *_data)
{
	struct MessageNewViewData *data = (struct MessageNewViewData *) _data;
	struct Window *win = data->win;

	data->cdata = calloc(1, sizeof(struct ContactListViewData));
	// FIXME: this is ugly
	data->cdata->view.win = win->win;

	g_debug("frame_contact_add_show()");

	window_layout_set(win, DEFAULT_THEME, "phoneui/messages/recipient_contact_add");

	window_text_set(win, "title", D_("Add a Contact"));

	data->bt1 = elm_button_add(win->win);
	elm_button_label_set(data->bt1, D_("Back"));
	evas_object_smart_callback_add(data->bt1, "clicked",
				       frame_contact_add_back_clicked, data);
	window_swallow(win, "button_back", data->bt1);
	evas_object_show(data->bt1);

	data->bt2 = elm_button_add(win->win);
	elm_button_label_set(data->bt2, D_("Add"));
	evas_object_smart_callback_add(data->bt2, "clicked",
				       frame_contact_add_add_clicked, data);
	window_swallow(win, "button_add", data->bt2);
	evas_object_show(data->bt2);

	elm_theme_extension_add(DEFAULT_THEME);

	contact_list_add(data->cdata);
	contact_list_fill(data->cdata);
}

static void
frame_contact_add_hide(void *_data)
{
	struct MessageNewViewData *data = (struct MessageNewViewData *) _data;

	g_debug("frame_contact_add_hide()");

	evas_object_del(data->bt1);
	evas_object_del(data->bt2);
	if (data->cdata)
		evas_object_del(data->cdata->list);
}

static void
frame_contact_add_back_clicked(void *_data, Evas_Object * obj, void *event_info)
{
	(void) obj;
	(void) event_info;
	struct MessageNewViewData *data = (struct MessageNewViewData *) _data;

	g_debug("frame_contact_add_back_clicked()");

	data->mode = MODE_RECIPIENT;
	window_frame_show(data->win, data, frame_recipient_show,
			  frame_recipient_hide);
}

static void
frame_contact_add_add_clicked(void *_data, Evas_Object * obj, void *event_info)
{
	(void) obj;
	(void) event_info;
	g_debug("frame_contact_add_add_clicked()");

	struct MessageNewViewData *data = (struct MessageNewViewData *) _data;
	Elm_Genlist_Item *it = elm_genlist_selected_item_get(data->cdata->list);
	GHashTable *properties = it ? (GHashTable *) elm_genlist_item_data_get(it) : NULL;

	if (properties) {
		char *str;
		GValue *tmp;
		str = phoneui_utils_contact_display_phone_get(properties);
		if (!str) {
			g_debug("contact needs a number to send a message ;)");
			return;
		}
		GHashTable *options = g_hash_table_new(g_str_hash, g_str_equal);
		g_hash_table_insert(options, "Phone",
				common_utils_new_gvalue_string(str));
		free(str);

		str = phoneui_utils_contact_display_name_get(properties);
		if (str) {
			g_hash_table_insert(options, "Name",
				common_utils_new_gvalue_string(str));
			free(str);
		}
		tmp = g_hash_table_lookup(properties, "Photo");
		if (tmp) {
			tmp = common_utils_new_gvalue_string(g_value_get_string(tmp));
			g_hash_table_insert(options, "Photo",
				tmp);
		}

		g_ptr_array_add(data->recipients, properties);
		data->mode = MODE_RECIPIENT;
		window_frame_show(data->win, data, frame_recipient_show,
				frame_recipient_hide);
	}
}



/* --- frame "number_add" --------------------------------------------------- */

static void
frame_number_add_show(void *_data)
{
	struct MessageNewViewData *data = (struct MessageNewViewData *) _data;
	struct Window *win = data->win;

	g_debug("frame_number_add_show()");

	window_layout_set(win, DEFAULT_THEME, "phoneui/messages/recipient_number_add");

	window_text_set(win, "title", D_("Add a Number"));

	data->bt1 = elm_button_add(window_evas_object_get(win));
	elm_button_label_set(data->bt1, D_("Back"));
	evas_object_smart_callback_add(data->bt1, "clicked",
				       frame_number_add_back_clicked, data);
	window_swallow(win, "button_back", data->bt1);
	evas_object_show(data->bt1);

	data->bt2 = elm_button_add(window_evas_object_get(win));
	elm_button_label_set(data->bt2, D_("Add"));
	evas_object_smart_callback_add(data->bt2, "clicked",
				       frame_number_add_add_clicked, data);
	window_swallow(win, "button_add", data->bt2);
	evas_object_show(data->bt2);

	data->entry = elm_entry_add(window_evas_object_get(win));
	evas_object_show(data->entry);
	elm_object_focus(data->entry);

	data->sc = elm_scroller_add(window_evas_object_get(win));
	elm_scroller_content_set(data->sc, data->entry);
	window_swallow(win, "entry", data->sc);
	evas_object_show(data->sc);

	window_kbd_show(win, KEYBOARD_NUMERIC);
}

static void
frame_number_add_hide(void *_data)
{
	struct MessageNewViewData *data = (struct MessageNewViewData *) _data;

	g_debug("frame_number_add_hide()");

	evas_object_del(data->bt1);
	evas_object_del(data->bt2);
	evas_object_del(data->sc);
	evas_object_del(data->entry);

	window_kbd_hide(data->win);
}

static void
frame_number_add_back_clicked(void *_data, Evas_Object * obj, void *event_info)
{
	(void) obj;
	(void) event_info;
	struct MessageNewViewData *data = (struct MessageNewViewData *) _data;

	g_debug("frame_number_add_back_clicked()");

	data->mode = MODE_RECIPIENT;
	window_frame_show(data->win, data, frame_recipient_show,
			  frame_recipient_hide);
}

static void
frame_number_add_add_clicked(void *_data, Evas_Object * obj, void *event_info)
{
	(void) obj;
	(void) event_info;
	struct MessageNewViewData *data = (struct MessageNewViewData *) _data;

	g_debug("frame_number_add_add_clicked()");

	char *number;
	number = g_strstrip(g_strdup(elm_entry_markup_to_utf8(elm_entry_entry_get(data->entry))));

	if (phone_utils_sms_is_valid_number(number)) {
		GHashTable *properties =
			g_hash_table_new(g_str_hash, g_str_equal);
		g_hash_table_insert(properties, "Name",
				    common_utils_new_gvalue_string("Number"));
		g_hash_table_insert(properties, "Phone",
				    common_utils_new_gvalue_string(number));
		g_hash_table_insert(properties, "Photo",
				common_utils_new_gvalue_string(CONTACT_NUMBER_PHOTO));
		g_ptr_array_add(data->recipients, properties);

		data->mode = MODE_RECIPIENT;
		window_frame_show(data->win, data, frame_recipient_show,
				  frame_recipient_hide);
	}
	if (number)
		free(number);
}


static void
frame_close_show(void *_data)
{
	struct MessageNewViewData *data = (struct MessageNewViewData *) _data;
	struct Window *win = data->win;

	g_debug("frame_close_show()");

	window_layout_set(win, DEFAULT_THEME, "phoneui/notification/close");

	data->information = elm_label_add(window_evas_object_get(win));
	elm_label_label_set(data->information,
			    D_("Do you really want to quit?"));
	window_swallow(win, "information", data->information);
	evas_object_show(data->information);

	data->bt1 = elm_button_add(window_evas_object_get(win));
	elm_button_label_set(data->bt1, D_("Yes"));
	evas_object_smart_callback_add(data->bt1, "clicked",
				       frame_close_yes_clicked, data);
	window_swallow(win, "button_yes", data->bt1);
	evas_object_show(data->bt1);

	data->bt2 = elm_button_add(window_evas_object_get(win));
	elm_button_label_set(data->bt2, D_("No"));
	evas_object_smart_callback_add(data->bt2, "clicked",
				       frame_close_no_clicked, data);
	window_swallow(win, "button_no", data->bt2);
	evas_object_show(data->bt2);
}
#endif

static void
_destroy_cb(struct View *_view)
{
	struct MessageNewViewData *view = (struct MessageNewViewData *)_view;
	g_debug("_destroy_cb");
	if (view->content) {
		free(view->content);
	}
	// TODO: properly free recipients

	g_debug("_destroy_cb DONE");
}

