
#include <phoneui/phoneui-utils.h>
#include <phone-utils.h>

#include "ui-utils.h"
#include "common-utils.h"
#include "widget/elm_keypad.h"
#include "message-new-view.h"
#include "views.h"
#include "phoneui-shr.h"


enum MessageNewModes {
	MODE_CONTENT,
	MODE_RECIPIENT,
	MODE_RECIPIENT_NUMBER,
	MODE_RECIPIENT_CONTACT,
	MODE_CLOSE
};

struct _recipient_pack {
	struct MessageNewViewData *view;
	GHashTable *recipient;
	Elm_Genlist_Item *it;
};

static Elm_Genlist_Item_Class itc;


static void _init_content_page(struct MessageNewViewData *view);
static void _init_recipient_page(struct MessageNewViewData *view);
static void _init_contacts_page(struct MessageNewViewData *view);
static void _init_number_page(struct MessageNewViewData *view);
static void _content_button_close_clicked(void *data, Evas_Object *obj, void *event_info);
static void _content_button_continue_clicked(void *data, Evas_Object *obj, void *event_info);
static void _content_changed(void *_data, Evas_Object * obj, void *event_info);
static void _recipients_button_back_clicked(void *data, Evas_Object *obj, void *event_info);
static void _recipients_button_add_contact_clicked(void *data, Evas_Object *obj, void *event_info);
static void _recipients_button_add_number_clicked(void *data, Evas_Object *obj, void *event_info);
static void _recipients_button_send_clicked(void *data, Evas_Object *obj, void *event_info);
static void _recipients_button_remove_clicked(void *data, Evas_Object *obj, void *event_info);
// static void _recipients_button_delete_clicked(void *_data, Evas_Object * obj, void *event_info);
static void _contacts_button_back_clicked(void *data, Evas_Object *obj, void *event_info);
static void _contacts_button_add_clicked(void *data, Evas_Object *obj, void *event_info);
static void _number_keypad_clicked(void *data, Evas_Object *obj, void *event_info);
static void _number_button_back_clicked(void *data, Evas_Object *obj, void *event_info);
static void _number_button_add_clicked(void *data, Evas_Object *obj, void *event_info);
static void _number_button_delete_clicked(void *data, Evas_Object *obj, void *event_info);
static void _number_update_number(struct MessageNewViewData* view);
static void _process_recipient(gpointer _properties, gpointer _data);
static void _contact_lookup(GHashTable *contact, gpointer data);
static char *gl_label_get(const void *data, Evas_Object * obj, const char *part);
static Evas_Object *gl_icon_get(const void *data, Evas_Object * obj, const char *part);
static void gl_del(const void *data, Evas_Object *obj);

static void _delete_cb(struct View *view, Evas_Object * win, void *event_info);
static void _destroy_cb(struct View *view);

struct MessageNewViewData *
message_new_view_init(GHashTable *options)
{
	struct MessageNewViewData *view;
	int ret;
	GValue *gval_tmp;
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
	view->contact_list_data.view = VIEW_PTR(*view);
	view->layout_content = NULL;
	view->layout_recipients = NULL;
	view->layout_contacts = NULL;
	view->layout_number = NULL;
	view->notify = NULL;
	if (options) {
		gval_tmp = g_hash_table_lookup(options, "Content");
		if (gval_tmp) {
			view->content = strdup(g_value_get_string(gval_tmp));
			g_hash_table_unref(options);
		}
		else {
			// FIXME: do we have to ref? or is that done by dbus for us?
			g_hash_table_ref(options);
			g_ptr_array_add(view->recipients, options);
		}
	}

	elm_theme_extension_add(phoneui_theme);
	win = ui_utils_view_window_get(VIEW_PTR(*view));
	ui_utils_view_delete_callback_set(VIEW_PTR(*view), _delete_cb);

	ui_utils_view_layout_set(VIEW_PTR(*view), phoneui_theme,
				 "phoneui/messages/new");

	view->pager = elm_pager_add(win);
	elm_win_resize_object_add(win, view->pager);
	ui_utils_view_swallow(VIEW_PTR(*view), "main", view->pager);
	evas_object_show(view->pager);

	_init_content_page(view);

	return view;
}


static void
_deinit_recipients_list(gpointer _properties, gpointer _data)
{
	GHashTable *properties;
	(void) _data;

	properties = (GHashTable *) _properties;
	g_hash_table_unref(properties);
}
void
message_new_view_deinit(struct MessageNewViewData *view)
{
	if (view) {
		ui_utils_view_deinit(VIEW_PTR(*view));
		g_ptr_array_foreach(view->recipients, _deinit_recipients_list, NULL);
		g_ptr_array_unref(view->recipients);
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
	char *label = NULL;
	struct _recipient_pack *pack = (struct _recipient_pack *)data;

	if (!strcmp(part, "elm.text")) {
		label = phoneui_utils_contact_display_name_get(pack->recipient);
		if (!label) {
			return strdup("Number");
		}
	}
	else if (!strcmp(part, "elm.text.sub")) {
		label = phoneui_utils_contact_display_phone_get(pack->recipient);
	}
	return label;
}

static Evas_Object *
gl_icon_get(const void *data, Evas_Object * obj, const char *part)
{
	struct _recipient_pack *pack = (struct _recipient_pack *)data;
	if (!strcmp(part, "elm.swallow.icon")) {
		const char *photo_file;
		GValue *tmp = g_hash_table_lookup(pack->recipient, "Photo");
		photo_file = (tmp) ? g_value_get_string(tmp) : CONTACT_DEFAULT_PHOTO;
		Evas_Object *photo = elm_icon_add(obj);
		elm_icon_file_set(photo, photo_file, NULL);
		evas_object_size_hint_aspect_set(photo,
						 EVAS_ASPECT_CONTROL_VERTICAL,
						 1, 1);
		return (photo);
	}

	if (!strcmp(part, "elm.swallow.end")) {
		Evas_Object *ico = elm_icon_add(obj);
		elm_icon_standard_set(ico, "delete");
		evas_object_smart_callback_add(ico, "clicked",
					       _recipients_button_remove_clicked,
					       pack);
		return ico;
	}

	return (NULL);
}

static void
gl_del(const void *data, Evas_Object *obj)
{
	(void) obj;
	struct _recipient_pack *pack = (struct _recipient_pack *)data;
	/* content of the pack will be freed by deinit */
	free(pack);
}


//static void message_send_callback(GError *error, int transaction_index, struct MessageNewViewData *data);

static void
_init_content_page(struct MessageNewViewData *view)
{
	Evas_Object *win, *btn, *sc;

	win = ui_utils_view_window_get(VIEW_PTR(*view));

	view->layout_content = elm_layout_add(view->pager);
	elm_win_resize_object_add(win, view->layout_content);
	elm_layout_file_set(view->layout_content, phoneui_theme,
			    "phoneui/messages/new/content");
	evas_object_show(view->layout_content);

	edje_object_part_text_set(elm_layout_edje_get(view->layout_content),
			"content_title", D_("Enter your message"));

	sc = elm_scroller_add(win);
	view->content_entry = elm_entry_add(win);
	evas_object_size_hint_weight_set(view->content_entry,
			EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_smart_callback_add(view->content_entry, "changed",
				       _content_changed, view);
	if (view->content != NULL)
		elm_entry_entry_set(view->content_entry, view->content);
	elm_scroller_content_set(sc, view->content_entry);
	evas_object_show(view->content_entry);
	elm_layout_content_set(view->layout_content, "content_entry", sc);
	evas_object_show(sc);
	elm_object_focus(view->content_entry);

	btn = elm_button_add(win);
	elm_button_label_set(btn, D_("Close"));
	evas_object_smart_callback_add(btn, "clicked",
				       _content_button_close_clicked, view);
	elm_layout_content_set(view->layout_content,
			       "content_button_close", btn);
	evas_object_show(btn);

	btn = elm_button_add(win);
	elm_button_label_set(btn, D_("Continue"));
	evas_object_smart_callback_add(btn, "clicked",
				       _content_button_continue_clicked, view);
	elm_layout_content_set(view->layout_content,
			       "content_button_continue", btn);
	evas_object_show(btn);

	elm_pager_content_push(view->pager, view->layout_content);
}

static void
_init_recipient_page(struct MessageNewViewData *view)
{
	Evas_Object *win, *btn;

	win = ui_utils_view_window_get(VIEW_PTR(*view));

	view->layout_recipients = elm_layout_add(view->pager);
	elm_win_resize_object_add(win, view->layout_recipients);
	elm_layout_file_set(view->layout_recipients, phoneui_theme,
			    "phoneui/messages/new/recipients");
	evas_object_show(view->layout_recipients);

	edje_object_part_text_set(elm_layout_edje_get(view->layout_recipients),
			"recipients_title", D_("Define Recipients"));

	view->list_recipients = elm_genlist_add(win);
	elm_genlist_horizontal_mode_set(view->list_recipients, ELM_LIST_LIMIT);
	evas_object_size_hint_align_set(view->list_recipients, 0.0, 0.0);
	evas_object_size_hint_weight_set(view->list_recipients,
					 EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	elm_object_scale_set(view->list_recipients, 1.0);
	elm_layout_content_set(view->layout_recipients, "recipients_list",
			       view->list_recipients);
	itc.item_style = "contact";
	itc.func.label_get = gl_label_get;
	itc.func.icon_get = gl_icon_get;
	itc.func.state_get = NULL;
	itc.func.del = gl_del;
	evas_object_show(view->list_recipients);

	g_ptr_array_foreach(view->recipients, _process_recipient, view);

	btn = elm_button_add(win);
	elm_button_label_set(btn, D_("Back"));
	evas_object_smart_callback_add(btn, "clicked",
				       _recipients_button_back_clicked, view);
	elm_layout_content_set(view->layout_recipients,
			       "recipients_button_back", btn);
	evas_object_show(btn);

	btn = elm_button_add(win);
	elm_button_label_set(btn, D_("Contact"));
	evas_object_smart_callback_add(btn, "clicked",
				       _recipients_button_add_contact_clicked,
				       view);
	elm_layout_content_set(view->layout_recipients,
			       "recipients_button_add_contact", btn);
	evas_object_show(btn);

	btn = elm_button_add(win);
	elm_button_label_set(btn, D_("Number"));
	evas_object_smart_callback_add(btn, "clicked",
				       _recipients_button_add_number_clicked,
				       view);
	elm_layout_content_set(view->layout_recipients,
			       "recipients_button_add_number", btn);
	evas_object_show(btn);

	btn = elm_button_add(win);
	elm_button_label_set(btn, D_("Send"));
	evas_object_smart_callback_add(btn, "clicked",
				       _recipients_button_send_clicked, view);
	elm_layout_content_set(view->layout_recipients,
			       "recipients_button_send", btn);
	evas_object_show(btn);

	elm_pager_content_push(view->pager, view->layout_recipients);
}

static void
_init_contacts_page(struct MessageNewViewData *view)
{
	Evas_Object *win, *btn;

	win = ui_utils_view_window_get(VIEW_PTR(*view));

	view->layout_contacts = elm_layout_add(view->pager);
	elm_win_resize_object_add(win, view->layout_contacts);
	elm_layout_file_set(view->layout_contacts, phoneui_theme,
			    "phoneui/messages/new/contacts");
	evas_object_show(view->layout_contacts);

	edje_object_part_text_set(elm_layout_edje_get(view->layout_contacts),
			"contacts_title", D_("Add Contact"));

	view->contact_list_data.layout = view->layout_contacts;
	contact_list_add(&view->contact_list_data);

	btn = elm_button_add(win);
	elm_button_label_set(btn, D_("Back"));
	evas_object_smart_callback_add(btn, "clicked",
				       _contacts_button_back_clicked, view);
	elm_layout_content_set(view->layout_contacts, "contacts_button_back", btn);
	evas_object_show(btn);

	btn = elm_button_add(win);
	elm_button_label_set(btn, D_("Add"));
	evas_object_smart_callback_add(btn, "clicked",
				       _contacts_button_add_clicked, view);
	elm_layout_content_set(view->layout_contacts, "contacts_button_add", btn);
	evas_object_show(btn);

	contact_list_fill(&view->contact_list_data);
	elm_pager_content_push(view->pager, view->layout_contacts);
}

static void
_init_number_page(struct MessageNewViewData *view)
{
	Evas_Object *win, *btn, *ico;

	win = ui_utils_view_window_get(VIEW_PTR(*view));

	view->number[0] = '\0';
	view->number_length = 0;
	view->layout_number = elm_layout_add(view->pager);
	elm_win_resize_object_add(win, view->layout_number);
	elm_layout_file_set(view->layout_number, phoneui_theme,
			    "phoneui/messages/new/number");
	evas_object_show(view->layout_number);

	edje_object_part_text_set(elm_layout_edje_get(view->layout_number),
			"number_title", D_("Add Number"));

	btn = elm_button_add(win);
	elm_button_label_set(btn, D_("Back"));
	evas_object_smart_callback_add(btn, "clicked",
				       _number_button_back_clicked, view);
	elm_layout_content_set(view->layout_number, "number_button_back", btn);
	evas_object_show(btn);

	btn = elm_button_add(win);
	elm_button_label_set(btn, D_("Add"));
	evas_object_smart_callback_add(btn, "clicked",
				       _number_button_add_clicked, view);
	elm_layout_content_set(view->layout_number, "number_button_add", btn);
	evas_object_show(btn);

	ico = elm_icon_add(win);
	evas_object_size_hint_aspect_set(ico, EVAS_ASPECT_CONTROL_VERTICAL, 1, 1);
	elm_icon_file_set(ico, phoneui_theme, "icon/edit-undo");
	evas_object_show(ico);

	btn = elm_button_add(win);
	elm_button_icon_set(btn, ico);
	elm_layout_content_set(view->layout_number, "number_button_delete", btn);
	evas_object_smart_callback_add(btn, "clicked",
				       _number_button_delete_clicked, view);
	evas_object_show(btn);

	view->number_keypad = (Evas_Object *) elm_keypad_add(win);
	evas_object_smart_callback_add(view->number_keypad, "clicked",
				       _number_keypad_clicked, view);
	elm_layout_content_set(view->layout_number, "number_keypad",
			       view->number_keypad);
	evas_object_show(view->number_keypad);

	elm_pager_content_push(view->pager, view->layout_number);
}

static void
_content_button_close_clicked(void *data, Evas_Object *obj, void *event_info)
{
	(void) obj;
	(void) event_info;
	struct MessageNewViewData *view = (struct MessageNewViewData *)data;
	// TODO: ask for confirmation
	message_new_view_deinit(view);
}

static void
_content_button_continue_clicked(void *data, Evas_Object *obj, void *event_info)
{
	(void) obj;
	(void) event_info;
	struct MessageNewViewData *view = (struct MessageNewViewData *)data;
	if (view->layout_recipients) {
		elm_pager_content_promote(view->pager, view->layout_recipients);
	}
	else {
		_init_recipient_page(view);
	}
}

static void
_recipients_button_back_clicked(void *data, Evas_Object *obj, void *event_info)
{
	(void) obj;
	(void) event_info;
	struct MessageNewViewData *view = (struct MessageNewViewData *)data;
	elm_pager_content_promote(view->pager, view->layout_content);
}

static void
_recipients_button_add_contact_clicked(void *data, Evas_Object *obj,
				       void *event_info)
{
	(void) obj;
	(void) event_info;
	struct MessageNewViewData *view = (struct MessageNewViewData *)data;
	if (view->layout_contacts) {
		elm_pager_content_promote(view->pager, view->layout_contacts);
	}
	else {
		_init_contacts_page(view);
	}
}

static void
_recipients_button_add_number_clicked(void *data, Evas_Object *obj,
				      void *event_info)
{
	(void) obj;
	(void) event_info;
	struct MessageNewViewData *view = (struct MessageNewViewData *)data;
	if (view->layout_number) {
		elm_pager_content_promote(view->pager, view->layout_number);
	}
	else {
		_init_number_page(view);
	}
}

static void
_recipients_button_remove_clicked(void *data, Evas_Object *obj, void *event_info)
{
	(void) obj;
	(void) event_info;
	struct _recipient_pack *pack = (struct _recipient_pack *)data;
	g_ptr_array_remove(pack->view->recipients, pack->recipient);
	elm_genlist_item_del(pack->it);
}

// static void
// _recipients_button_delete_clicked(void *_data, Evas_Object * obj, void *event_info)
// {
// 	(void) obj;
// 	(void) event_info;
// 	struct MessageNewViewData *data = (struct MessageNewViewData *) _data;
//
// 	Elm_Genlist_Item *it =
// 			elm_genlist_selected_item_get(data->list_recipients);
// 	if (it) {
// 		GHashTable *parameters = (GHashTable *) elm_genlist_item_data_get(it);
// 		g_ptr_array_remove(data->recipients, parameters);
// 		elm_genlist_item_del(it);
// 	}
// }

static void
_recipients_button_send_clicked(void *data, Evas_Object *obj, void *event_info)
{
	(void) obj;
	(void) event_info;
	struct MessageNewViewData *view = (struct MessageNewViewData *)data;
	if (view->recipients->len) {
		// TODO: show inwin for progress via callback
		phoneui_utils_sms_send(view->content, view->recipients, NULL, NULL);
		message_new_view_deinit(view);
		free(view);
	}
}

static void
_contacts_button_back_clicked(void *data, Evas_Object *obj, void *event_info)
{
	(void) obj;
	(void) event_info;
	struct MessageNewViewData *view = (struct MessageNewViewData *)data;
	elm_pager_content_promote(view->pager, view->layout_recipients);
}

static void
_contacts_button_add_clicked(void *data, Evas_Object *obj, void *event_info)
{
	(void) obj;
	(void) event_info;
	struct MessageNewViewData *view = (struct MessageNewViewData *)data;
	Elm_Genlist_Item *it;
	GHashTable *properties, *options;

	it = elm_genlist_selected_item_get(view->contact_list_data.list);
	properties = it ? (GHashTable *) elm_genlist_item_data_get(it) : NULL;
	if (properties) {
		char *str;
		GValue *tmp;
		str = phoneui_utils_contact_display_phone_get(properties);
		if (!str) {
			g_debug("contact needs a number to send a message ;)");
			return;
		}
		options = g_hash_table_new_full(g_str_hash, g_str_equal,
						NULL, common_utils_gvalue_free);
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
			tmp = common_utils_new_gvalue_string(
					g_value_get_string(tmp));
			g_hash_table_insert(options, "Photo", tmp);
		}
		g_ptr_array_add(view->recipients, options);
		_process_recipient(options, view);
	}
	elm_pager_content_promote(view->pager, view->layout_recipients);
}

static void
_number_keypad_clicked(void *data, Evas_Object *obj, void *event_info)
{
	(void) obj;
	struct MessageNewViewData *view = (struct MessageNewViewData *)data;
	char input = ((char *) event_info)[0];

	if (view->number_length < 64) {
		view->number[view->number_length] = input;
		view->number[view->number_length+1] = '\0';
		view->number_length++;
		_number_update_number(view);
	}
}

static void
_number_button_back_clicked(void *data, Evas_Object *obj, void *event_info)
{
	(void) obj;
	(void) event_info;
	struct MessageNewViewData *view = (struct MessageNewViewData *)data;
	elm_pager_content_promote(view->pager, view->layout_recipients);
}

static void
_number_button_add_clicked(void *data, Evas_Object *obj, void *event_info)
{
	(void) obj;
	(void) event_info;
	struct MessageNewViewData *view = (struct MessageNewViewData *)data;

	if (phone_utils_sms_is_valid_number(view->number)) {
		GHashTable *properties =
			g_hash_table_new_full(g_str_hash, g_str_equal,
					 NULL, common_utils_gvalue_free);
		g_hash_table_insert(properties, "Name",
				    common_utils_new_gvalue_string("Number"));
		g_hash_table_insert(properties, "Phone",
				    common_utils_new_gvalue_string(view->number));
		g_hash_table_insert(properties, "Photo",
				common_utils_new_gvalue_string(CONTACT_NUMBER_PHOTO));
		g_ptr_array_add(view->recipients, properties);
		_process_recipient(properties, view);
		view->number[0] = '\0';
		view->number_length = 0;
		_number_update_number(view);
		elm_pager_content_promote(view->pager, view->layout_recipients);
	}
	else {
		if (!view->notify) {
			view->notify = ui_utils_notify(ui_utils_view_window_get(VIEW_PTR(*view)), D_("You have entered<br>an invalid number."), 5);
		}
		evas_object_show(view->notify);
	}
}

static void
_number_button_delete_clicked(void *data, Evas_Object *obj, void *event_info)
{
	(void) obj;
	(void) event_info;
	struct MessageNewViewData *view = (struct MessageNewViewData *)data;
	if (view->number_length > 0) {
		view->number_length--;
		view->number[view->number_length] = '\0';
		_number_update_number(view);
	}
}

static void
_number_update_number(struct MessageNewViewData *view)
{
	g_debug("Updating number to %s", view->number);
	edje_object_part_text_set(elm_layout_edje_get(view->layout_number),
			"number_number", view->number);
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
		edje_object_part_text_set(elm_layout_edje_get
			(view->layout_content), "characters_left", text);
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
	edje_object_part_text_set(elm_layout_edje_get(view->layout_content),
				  "characters_left", text);
	if (view->content) {
		free(view->content);
	}
	view->content = content;
}

static void
_process_recipient(gpointer _properties, gpointer _data)
{
	GHashTable *properties;
	struct MessageNewViewData *view;
	struct _recipient_pack *pack;
	GValue *gval_tmp;

	properties = (GHashTable *) _properties;
	view = (struct MessageNewViewData *) _data;
	pack = malloc(sizeof(struct _recipient_pack));
        pack->recipient = properties;
	pack->view = view;
	pack->it = elm_genlist_item_append(view->list_recipients, &itc, pack,
					   NULL, ELM_GENLIST_ITEM_NONE,
					   NULL, NULL);
	/* try to resolve the number to a contact */
	gval_tmp = g_hash_table_lookup(properties, "Phone");
	if (gval_tmp) {
		phoneui_utils_contact_lookup(g_value_get_string(gval_tmp),
					     _contact_lookup, pack);
	}
}

static void
_contact_lookup(GHashTable *contact, gpointer data)
{
	char *tmp;
	const char *tmp2;
	GValue *gval_tmp;
	struct _recipient_pack *pack;

	if (contact == NULL)
		return;

	pack = (struct _recipient_pack *)data;
	tmp = phoneui_utils_contact_display_name_get(contact);
	if (tmp) {
		g_hash_table_insert(pack->recipient, "Name",
				    common_utils_new_gvalue_string(tmp));
		free(tmp);
	}
	gval_tmp = g_hash_table_lookup(contact, "Photo");
	if (gval_tmp) {
		tmp2 = g_value_get_string(gval_tmp);
		g_hash_table_insert(pack->recipient, "Photo",
				    common_utils_new_gvalue_string(tmp2));
	}
	if (pack->view->layout_recipients) {
		elm_genlist_item_update(pack->it);
	}
}

static void
_delete_cb(struct View *view, Evas_Object * win, void *event_info)
{
	(void)win;
	(void)event_info;
	message_new_view_deinit((struct MessageNewViewData *)view);
	free(view);
}

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

