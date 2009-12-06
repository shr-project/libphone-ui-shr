#include "views.h"

#include <phoneui/phoneui-utils.h>
#include <phone-utils.h>

#include "util/common-utils.h"

/* FIXME: HACKS FROM elm_priv.h that should be removed */
#if 1
void         elm_widget_scale_set(Evas_Object *obj, double scale);
void         elm_widget_focus_set(Evas_Object *obj, int first);
#endif


enum MessageNewModes {
	MODE_CONTENT,
	MODE_RECIPIENT,
	MODE_RECIPIENT_NUMBER,
	MODE_RECIPIENT_CONTACT,
	MODE_CLOSE
};

struct MessageNewViewData {
	struct Window *win;
	int mode;
	char *content;
	Evas_Object *bb, *entry, *bt1, *bt2, *bt3, *bt4, *bt5, *hv, *bx, *hbt1,
		*hbt2, *hbt3, *sc;
	Evas_Object *list_contacts;
	Evas_Object *list_recipients;
	Evas_Object *information;

	GPtrArray *recipients;

	struct ContactListViewData *cdata;

	int messages_sent;
};

static Elm_Genlist_Item_Class itc;


static char *
gl_label_get(const void *data, Evas_Object * obj, const char *part)
{
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

	return (label);
}

static Evas_Object *
gl_icon_get(const void *data, Evas_Object * obj, const char *part)
{
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
  frame_content_show(void *_data);
static void
  frame_content_hide(void *_data);
static void
  frame_content_close_clicked(void *_data, Evas_Object * obj, void *event_info);
static void
frame_content_continue_clicked(void *_data, Evas_Object * obj,
			       void *event_info);
static void
frame_content_content_changed(void *_data, Evas_Object * obj, void *event_info);

static void
  frame_recipient_show(void *_data);
static void
  frame_recipient_hide(void *_data);
static void
 frame_recipient_back_clicked(void *_data, Evas_Object * obj, void *event_info);
static void
frame_recipient_contact_add_clicked(void *_data, Evas_Object * obj,
				    void *event_info);
static void
frame_recipient_number_add_clicked(void *_data, Evas_Object * obj,
				   void *event_info);
static void
frame_recipient_delete_clicked(void *_data, Evas_Object * obj,
			       void *event_info);
static void
frame_recipient_continue_clicked(void *_data, Evas_Object * obj,
				 void *event_info);
static void
  frame_recipient_process_recipient(gpointer _properties, gpointer _data);
static void
frame_recipient_send_callback(GError * error, int transaction_index,
			      const char *timestamp, void *data);

static void
  frame_contact_add_show(void *_data);
static void
  frame_contact_add_hide(void *_data);
static void
frame_contact_add_back_clicked(void *_data, Evas_Object * obj,
			       void *event_info);
static void
frame_contact_add_add_clicked(void *_data, Evas_Object * obj, void *event_info);

static void
  frame_number_add_show(void *_data);
static void
  frame_number_add_hide(void *_data);
static void
 frame_number_add_add_clicked(void *_data, Evas_Object * obj, void *event_info);
static void
frame_number_add_back_clicked(void *_data, Evas_Object * obj, void *event_info);

static void
  frame_close_show(void *_data);
static void
  frame_close_hide(void *_data);
static void
  frame_close_yes_clicked(void *_data, Evas_Object * obj, void *event_info);
static void
  frame_close_no_clicked(void *_data, Evas_Object * obj, void *event_info);

static void
  frame_sending_show(void *_data);
static void
  frame_sending_hide(void *_data);



/* --- main message new view functions -------------------------------------------- */

void *
message_new_view_show(struct Window *win, void *_options)
{
	GHashTable *options = (GHashTable *) _options;

	g_debug("message_new_view_show()");

	struct MessageNewViewData *data =
		calloc(1, sizeof(struct MessageNewViewData));

	data->win = win;
	data->mode = MODE_CONTENT;
	data->content = NULL;
	data->recipients = g_ptr_array_new();
	data->messages_sent = 0;
	data->cdata = NULL;

	if (options) {
		g_ptr_array_add(data->recipients, options);
	}


	window_frame_show(win, data, frame_content_show, frame_content_hide);
	window_show(win);

	return data;
}

void
message_new_view_hide(void *_data)
{
	g_debug("message_new_view_hide()");
	free(_data);
}

//static void message_send_callback(GError *error, int transaction_index, struct MessageNewViewData *data) 
//{
//    g_debug("message_send_callback()");
//}
//


/* --- frame "content" ------------------------------------------------------------ */

static void
frame_content_show(void *_data)
{
	struct MessageNewViewData *data = (struct MessageNewViewData *) _data;
	struct Window *win = data->win;

	g_debug("frame_content_show()");

	window_layout_set(win, DEFAULT_THEME, "phoneui/messages/content_edit");

	data->bt1 = elm_button_add(win->win);
	elm_button_label_set(data->bt1, D_("Close"));
	evas_object_smart_callback_add(data->bt1, "clicked",
				       frame_content_close_clicked, data);
	window_swallow(win, "button_close", data->bt1);
	evas_object_show(data->bt1);

	data->bt2 = elm_button_add(window_evas_object_get(win));
	elm_button_label_set(data->bt2, D_("Continue"));
	evas_object_smart_callback_add(data->bt2, "clicked",
				       frame_content_continue_clicked, data);
	window_swallow(win, "button_continue", data->bt2);
	evas_object_show(data->bt2);

	data->sc = elm_scroller_add(window_evas_object_get(win));
	data->entry = elm_entry_add(window_evas_object_get(win));
	evas_object_size_hint_weight_set(data->entry,
			EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_smart_callback_add(data->entry, "changed",
				       frame_content_content_changed, data);
	if (data->content != NULL)
		elm_entry_entry_set(data->entry, data->content);
	elm_scroller_content_set(data->sc, data->entry);
	evas_object_show(data->entry);

	window_swallow(win, "entry", data->sc);
	evas_object_show(data->sc);

	elm_object_focus(data->entry);
}

static void
frame_content_hide(void *_data)
{
	struct MessageNewViewData *data = (struct MessageNewViewData *) _data;
	struct Window *win = data->win;

	g_debug("frame_content_hide()");

	// Free objects
	evas_object_del(data->bt1);
	evas_object_del(data->bt2);
	evas_object_del(data->entry);
	evas_object_del(data->sc);

	window_kbd_hide(win);
}

static void
frame_content_close_clicked(void *_data, Evas_Object * obj, void *event_info)
{
	struct MessageNewViewData *data = (struct MessageNewViewData *) _data;

	g_debug("frame_content_close_clicked()");

	window_frame_show(data->win, data, frame_close_show, frame_close_hide);
}

static void
frame_content_continue_clicked(void *_data, Evas_Object * obj, void *event_info)
{
	struct MessageNewViewData *data = (struct MessageNewViewData *) _data;

	g_debug("frame_content_continue_clicked()");

	data->mode = MODE_RECIPIENT;
	window_frame_show(data->win, data, frame_recipient_show,
			  frame_recipient_hide);
}

static void
frame_content_content_changed(void *_data, Evas_Object * obj, void *event_info)
{
	struct MessageNewViewData *data = (struct MessageNewViewData *) _data;
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
		window_text_set(data->win, "characters_left", text);
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
	window_text_set(data->win, "characters_left", text);
	if (data->content) {
		free(data->content);
	}
	data->content = content;
}



/* --- frame "recipient" ---------------------------------------------------- */

static void
frame_recipient_show(void *_data)
{
	struct MessageNewViewData *data = (struct MessageNewViewData *) _data;
	struct Window *win = data->win;

	g_debug("frame_recipient_show()");

	window_layout_set(win, DEFAULT_THEME, "phoneui/messages/recipient_edit");

	window_text_set(win, "title", D_("Define Recipients"));

	data->bt1 = elm_button_add(window_evas_object_get(win));
	elm_button_label_set(data->bt1, D_("Back"));
	evas_object_smart_callback_add(data->bt1, "clicked",
				       frame_recipient_back_clicked, data);
	window_swallow(win, "button_back", data->bt1);
	evas_object_show(data->bt1);

	data->bt2 = elm_button_add(window_evas_object_get(win));
	elm_button_label_set(data->bt2, D_("Send"));
	evas_object_smart_callback_add(data->bt2, "clicked",
				       frame_recipient_continue_clicked, data);
	window_swallow(win, "button_continue", data->bt2);
	evas_object_show(data->bt2);

	data->bt3 = elm_button_add(window_evas_object_get(win));
	elm_button_label_set(data->bt3, D_("Add Contact"));
	evas_object_smart_callback_add(data->bt3, "clicked",
				       frame_recipient_contact_add_clicked,
				       data);
	window_swallow(win, "button_contact_add", data->bt3);
	evas_object_show(data->bt3);

	data->bt4 = elm_button_add(window_evas_object_get(win));
	elm_button_label_set(data->bt4, D_("Add Number"));
	evas_object_smart_callback_add(data->bt4, "clicked",
				       frame_recipient_number_add_clicked,
				       data);
	window_swallow(win, "button_number_add", data->bt4);
	evas_object_show(data->bt4);

	data->bt5 = elm_button_add(window_evas_object_get(win));
	elm_button_label_set(data->bt5, D_("Remove"));
	evas_object_smart_callback_add(data->bt5, "clicked",
				       frame_recipient_delete_clicked, data);
	window_swallow(win, "button_delete", data->bt5);
	evas_object_show(data->bt5);

	elm_theme_extension_add(DEFAULT_THEME);
	data->list_recipients = elm_genlist_add(window_evas_object_get(win));
	elm_genlist_horizontal_mode_set(data->list_recipients, ELM_LIST_LIMIT);
	evas_object_size_hint_align_set(data->list_recipients, 0.0, 0.0);
	elm_widget_scale_set(data->list_recipients, 1.0);
	window_swallow(data->win, "list", data->list_recipients);
	itc.item_style = "contact";
	itc.func.label_get = gl_label_get;
	itc.func.icon_get = gl_icon_get;
	itc.func.state_get = NULL;
	itc.func.del = NULL;
	evas_object_show(data->list_recipients);

	g_ptr_array_foreach(data->recipients, frame_recipient_process_recipient,
			    data);
}

static void
frame_recipient_hide(void *_data)
{
	struct MessageNewViewData *data = (struct MessageNewViewData *) _data;
	struct Window *win = data->win;

	g_debug("frame_recipient_hide()");

	evas_object_del(data->bt1);
	evas_object_del(data->bt2);
	evas_object_del(data->bt3);
	evas_object_del(data->bt4);
	evas_object_del(data->bt5);
	evas_object_del(data->list_recipients);
}

static void
frame_recipient_back_clicked(void *_data, Evas_Object * obj, void *event_info)
{
	struct MessageNewViewData *data = (struct MessageNewViewData *) _data;

	g_debug("frame_recipient_back_clicked()");

	data->mode = MODE_CONTENT;
	window_frame_show(data->win, data, frame_content_show,
			  frame_content_hide);
}

static void
frame_recipient_contact_add_clicked(void *_data, Evas_Object * obj,
				    void *event_info)
{
	struct MessageNewViewData *data = (struct MessageNewViewData *) _data;

	g_debug("frame_recipient_contact_add_clicked()");

	data->mode = MODE_RECIPIENT_CONTACT;
	window_frame_show(data->win, data, frame_contact_add_show,
			  frame_contact_add_hide);
}

static void
frame_recipient_number_add_clicked(void *_data, Evas_Object * obj,
				   void *event_info)
{
	struct MessageNewViewData *data = (struct MessageNewViewData *) _data;

	g_debug("frame_recipient_number_add_clicked()");

	data->mode = MODE_RECIPIENT_NUMBER;
	window_frame_show(data->win, data, frame_number_add_show,
			  frame_number_add_hide);
}

static void
frame_recipient_delete_clicked(void *_data, Evas_Object * obj, void *event_info)
{
	struct MessageNewViewData *data = (struct MessageNewViewData *) _data;

	g_debug("frame_recipient_delete_clicked()");

	Elm_Genlist_Item *it = elm_genlist_selected_item_get(data->list_recipients);
	if (it) {
		GHashTable *parameters = elm_genlist_item_data_get(it);
		g_ptr_array_remove(data->recipients, parameters);
		elm_genlist_item_del(it);
	}
}

static void
frame_recipient_continue_clicked(void *_data, Evas_Object * obj,
				 void *event_info)
{
	struct MessageNewViewData *data = (struct MessageNewViewData *) _data;

	g_debug("frame_recipient_continue_clicked()");

	if (data->recipients->len) {
		window_frame_show(data->win, data, frame_sending_show,
				  frame_sending_hide);

		phoneui_utils_sms_send(data->content, data->recipients, NULL, NULL);

		window_destroy(data->win, NULL);
	}
}

static void
frame_recipient_send_callback(GError * error, int transaction_index,
			      const char *timestamp, void *_data)
{
	struct MessageNewViewData *data = (struct MessageNewViewData *) _data;
	data->messages_sent++;
	if (data->messages_sent == data->recipients->len) {
		sleep(1);
		window_destroy(data->win, NULL);
	}
}


static void
frame_recipient_process_recipient(gpointer _properties, gpointer _data)
{
	GHashTable *properties = (GHashTable *) _properties;
	//g_debug("adding recipient %s", g_hash_table_lookup(properties, "name"));
	struct MessageNewViewData *data = (struct MessageNewViewData *) _data;

	elm_genlist_item_append(data->list_recipients, &itc, properties,
		NULL, ELM_GENLIST_ITEM_NONE, NULL, NULL);
}



/* --- frame "contact_add" -------------------------------------------------- */

static void
frame_contact_add_show(void *_data)
{
	struct MessageNewViewData *data = (struct MessageNewViewData *) _data;
	struct Window *win = data->win;

	data->cdata = calloc(1, sizeof(struct ContactListViewData));
	data->cdata->win = data->win;

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
	struct MessageNewViewData *data = (struct MessageNewViewData *) _data;

	g_debug("frame_contact_add_back_clicked()");

	data->mode = MODE_RECIPIENT;
	window_frame_show(data->win, data, frame_recipient_show,
			  frame_recipient_hide);
}

static void
frame_contact_add_add_clicked(void *_data, Evas_Object * obj, void *event_info)
{
	g_debug("frame_contact_add_add_clicked()");

	struct MessageNewViewData *data = (struct MessageNewViewData *) _data;
	Elm_Genlist_Item *it = elm_genlist_selected_item_get(data->cdata->list);
	GHashTable *properties = it ? elm_genlist_item_data_get(it) : NULL;

	if (properties) {
		const char *photo;
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
	elm_widget_focus_set(data->entry, 1);

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
	struct MessageNewViewData *data = (struct MessageNewViewData *) _data;

	g_debug("frame_number_add_back_clicked()");

	data->mode = MODE_RECIPIENT;
	window_frame_show(data->win, data, frame_recipient_show,
			  frame_recipient_hide);
}

static void
frame_number_add_add_clicked(void *_data, Evas_Object * obj, void *event_info)
{
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


/* --- frame "close" -------------------------------------------------------- */

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

static void
frame_close_hide(void *_data)
{
	struct MessageNewViewData *data = (struct MessageNewViewData *) _data;

	g_debug("frame_close_hide()");

	evas_object_del(data->bt1);
	evas_object_del(data->bt2);
	evas_object_del(data->information);
}

static void
frame_close_yes_clicked(void *_data, Evas_Object * obj, void *event_info)
{
	struct MessageNewViewData *data = (struct MessageNewViewData *) _data;

	g_debug("frame_close_yes_clicked()");

	window_destroy(data->win, NULL);
}

static void
frame_close_no_clicked(void *_data, Evas_Object * obj, void *event_info)
{
	struct MessageNewViewData *data = (struct MessageNewViewData *) _data;

	g_debug("frame_close_no_clicked()");

	window_frame_show(data->win, data, frame_content_show,
			  frame_content_hide);
}



/* --- frame "sending" ------------------------------------------------------ */

static void
frame_sending_show(void *_data)
{
	struct MessageNewViewData *data = (struct MessageNewViewData *) _data;

	g_debug("frame_sending_show()");

	window_layout_set(data->win, DEFAULT_THEME, "phoneui/messages/sending");
	window_text_set(data->win, "text", D_("Sending.."));
}

static void
frame_sending_hide(void *data)
{
	g_debug("frame_sending_hide()");
}
