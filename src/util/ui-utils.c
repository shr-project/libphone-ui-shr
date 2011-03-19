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
#include <Evas.h>

#include "ui-utils.h"
#include "views.h"

/* FIXME: hackish, remove */
#include "phoneui-shr.h"

/* already defined in window.h - reactivate
when that beast got finally removed */
// struct InwinButton {
// 	char *label;
// 	void (*callback) (void *, Evas_Object *, void *);
// };


struct View *
ui_utils_view_new(const char *title)
{
	(void) title;
	struct View *view;

	view = calloc(1, sizeof(struct View));
	if (!view) {
		g_critical("Failed to allocate memory (%s:%d)", __FUNCTION__, __LINE__);
		return NULL;
	}

	return view;
}

int
ui_utils_view_init(struct View *view, Elm_Win_Type type, const char *title,
			void (*show_cb) (struct View *view),
		 	void (*hide_cb) (struct View *view),
		 	void (*destroy_cb)(struct View *view))
{
	int ret = 0;
	g_debug("Initializing window with title: %s", title);

	if (!view) {
		g_critical("struct View is NULL (%s:%d)", __FUNCTION__, __LINE__);
		ret = 1;
		goto end;
	}

	/* Window */
	view->win = elm_win_add(NULL, "main", type);
	if (!view->win) {
		g_critical("Wasn't able to create a window (%s:%d)", __FUNCTION__, __LINE__);
		ret = 1;
		goto end;
	}

	elm_win_title_set(view->win, title);

	elm_win_autodel_set(view->win, 0);
	if (phoneui_theme) {
		elm_theme_overlay_add(NULL, phoneui_theme);
	}


	/* Background */
	view->background = elm_bg_add(view->win);
	if (!view->background) {
		g_critical("elm_bg_add failed (%s:%d)", __FUNCTION__, __LINE__);
		ret = 1;
		goto free_win;
	}
	evas_object_size_hint_weight_set(view->background, 1.0, 1.0);
	elm_win_resize_object_add(view->win, view->background);
	evas_object_show(view->background);

	/* Layout */
	view->layout = elm_layout_add(view->win);
	if (!view->layout) {
		g_critical("elm_layout_add failed (%s:%d)", __FUNCTION__, __LINE__);
		ret = 1;
		goto free_bg;
	}
	evas_object_size_hint_weight_set(view->layout, 1.0, 1.0);
	elm_win_resize_object_add(view->win, view->layout);
	evas_object_show(view->layout);

	/* FIXME: Not perfect, should probably be set from a config wether we want max or resize */
	evas_object_size_hint_min_set(view->win, 100, 200);
	elm_win_maximized_set(view->win, EINA_TRUE);

	view->show_cb = show_cb;
	view->hide_cb = hide_cb;
	view->destroy_cb = destroy_cb;
	/* END of wtf */
end:
	return ret;

/* Error handling */
free_bg:
	evas_object_del(view->background);
free_win:
	evas_object_del(view->win);

	goto end;
}

int
ui_utils_view_is_visible(struct View *view)
{
	return evas_object_visible_get(view->win);
}

void
ui_utils_view_show(struct View *view)
{
	if (view->win) {
		if (view->show_cb) {
			view->show_cb(view);
		}
		evas_object_show(view->win);
		elm_win_activate(view->win);
	}
	else {
		g_critical("Window is NULL (%s:%d)", __FUNCTION__, __LINE__);
	}
}

void
ui_utils_view_hide(struct View *view)
{
	if (!view->win) {
		g_critical("Window is NULL (%s:%d)", __FUNCTION__, __LINE__);
		return;
	}
	if (view->hide_cb) {
		view->hide_cb(view);
	}
	evas_object_hide(view->win);
}

void
ui_utils_view_toggle(struct View *view)
{
	if (ui_utils_view_is_visible(view)) {
		ui_utils_view_hide(view);
	}
	else {
		ui_utils_view_show(view);
	}
}

void
ui_utils_view_layout_set(struct View *view, const char *file, const char *part)
{
	if (!view) {
		g_critical("Window is NULL (%s:%d)", __FUNCTION__, __LINE__);
		return;
	}
	g_debug("setting layout from file '%s' (%s)", file, part);
	elm_layout_file_set(view->layout, file, part);
}

Evas_Object *
ui_utils_view_layout_get(struct View *view)
{
	if (!view) {
		g_critical("Window is NULL (%s:%d)", __FUNCTION__, __LINE__);
		return NULL;
	}
	return elm_layout_edje_get(view->layout);
}

Evas_Object *
ui_utils_view_window_get(struct View *view)
{
	if (!view) {
		g_critical("Window is NULL (%s:%d)", __FUNCTION__, __LINE__);
		return NULL;
	}
	return view->win;
}

void
ui_utils_view_delete_callback_set(struct View *view,
			   void (*cb) (struct View *viewdow, Evas_Object *view,
				       void *event_info))
{
	if (!view) {
		g_critical("Window is NULL (%s:%d)", __FUNCTION__, __LINE__);
		return;
	}
	if (!cb) {
		g_critical("Tried to set cb to NULL (%s:%d)", __FUNCTION__, __LINE__);
		return;
	}
	evas_object_smart_callback_add(view->win, "delete,request", (Evas_Smart_Cb) cb, view);
}

void
ui_utils_view_text_set(struct View *view, const char *key, const char *value)
{
	if (!view) {
		g_critical("Window is NULL (%s:%d)", __FUNCTION__, __LINE__);
		return;
	}
	edje_object_part_text_set(elm_layout_edje_get(view->layout), key, value);
}

void
ui_utils_view_swallow(struct View *view, const char *key, Evas_Object * object)
{
	if (!view) {
		g_critical("Window is NULL (%s:%d)", __FUNCTION__, __LINE__);
		return;
	}
	/* What's this: ?
	 * edje_object_part_swallow(elm_layout_edje_get(view->layout), key, object);
	 */
	elm_layout_content_set(view->layout, key, object);
}

void
ui_utils_view_unswallow(struct View *view, Evas_Object * object)
{
	if (!view) {
		g_critical("Window is NULL (%s:%d)", __FUNCTION__, __LINE__);
		return;
	}
	edje_object_part_unswallow(elm_layout_edje_get(view->layout), object);
}

void
ui_utils_view_deinit(struct View *view)
{
	if (!view) {
		g_critical("struct View is NULL (%s:%d)", __FUNCTION__, __LINE__);
		return;
	}

	g_debug("Calling view destroy callback");
	if (view->destroy_cb)
		view->destroy_cb(view);
	g_debug("View destroy callback DONE");

	ui_utils_view_hide(view);
	evas_object_del(view->win);

	/* This is enough to mark it's empty. */
	view->win = NULL;
	view->show_cb = NULL;
	view->hide_cb = NULL;
	view->destroy_cb = NULL;
	g_debug("Deinit of View done");
}

/* Think about this a bit more... Looks sane */
Evas_Object *
ui_utils_view_inwin_dialog(struct View *view, const char *label, GList *buttons,
		    		void *data)
{
	Evas_Object *inwin = elm_win_inwin_add(ui_utils_view_window_get(view));

	Evas_Object *bx = elm_box_add(ui_utils_view_window_get(view));
	elm_box_homogenous_set(bx, 1);

	Evas_Object *e = elm_label_add(ui_utils_view_window_get(view));
	elm_label_label_set(e, label);
	evas_object_show(e);
	elm_box_pack_end(bx, e);

	Evas_Object *bx2 = elm_box_add(ui_utils_view_window_get(view));
	elm_box_horizontal_set(bx2, 1);
	elm_box_homogenous_set(bx2, 1);

	for (buttons = g_list_first(buttons); buttons;
	     buttons = g_list_next(buttons)) {
		e = elm_button_add(ui_utils_view_window_get(view));
		elm_button_label_set(e,
				     ((struct InwinButton *) buttons->data)->
				     label);
		evas_object_smart_callback_add(e, "clicked",
					       ((struct InwinButton *)
						buttons->data)->callback, data);
		evas_object_show(e);
		elm_box_pack_end(bx2, e);
		g_free(buttons->data);
	}

	g_list_free(buttons);

	evas_object_show(bx2);
	elm_box_pack_end(bx, bx2);
	evas_object_show(bx);

	elm_win_inwin_content_set(inwin, bx);

	evas_object_show(inwin);

	return (inwin);
}

struct _dialog_pack {
	void (*callback)(int, void *);
	void *data;
	Evas_Object *inwin;
};

/*FIXME: clean all the _dialog_pack */
static void
_inwin_dialog_cb(void *data, Evas_Object *obj, void *event_info)
{
	(void) event_info;
	struct _dialog_pack *pack = (struct _dialog_pack *)data;
	int *tmp;
	int cb_type = 0;

	if ((tmp = evas_object_data_get(obj, "type")))
		cb_type = GPOINTER_TO_INT(tmp);

	if (pack->callback)
		pack->callback(cb_type, pack->data);
	evas_object_del(pack->inwin);
}

void
ui_utils_dialog(struct View *view, const char *label, int buttonflags,
		      void (*callback)(int, void *), void *data)
{
	ui_utils_dialog_check(view, label, NULL, NULL, buttonflags, callback, data);
}

void
ui_utils_dialog_check(struct View *view, const char *label, const char *check_label,
		      Eina_Bool *check_value, int buttonflags,
		      void (*callback)(int, void *), void *data)
{
	Evas_Object *win, *box, *box2, *lbl, *btn, *check;
	char *tmp;
	struct _dialog_pack *pack = malloc(sizeof(struct _dialog_pack));
	win = ui_utils_view_window_get(view);
	pack->callback = callback;
	pack->data = data;
	pack->inwin = elm_win_inwin_add(win);
	box = elm_box_add(win);
	evas_object_size_hint_align_set(box, EVAS_HINT_FILL, EVAS_HINT_FILL);
	evas_object_size_hint_weight_set(box, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);

	if (strlen(label)  // workaround segfault
		tmp = g_strdup_printf("<font align=center>%s</font>", label);
	else 
		tmp = g_strdup_printf("<font align=center>Dialog</font>");
	lbl = elm_label_add(win);
	elm_label_label_set(lbl, tmp);
	elm_label_line_wrap_set(lbl, EINA_TRUE);
	evas_object_size_hint_align_set(lbl, EVAS_HINT_FILL, EVAS_HINT_FILL);
	evas_object_size_hint_weight_set(lbl, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	g_free(tmp);

	evas_object_show(lbl);
	elm_box_pack_end(box, lbl);

	if 	(check_label && strlen(check_label) && check_value) {
		check = elm_check_add(win);
		elm_check_label_set(check, check_label);
		elm_check_state_set(check, *check_value);
		elm_check_state_pointer_set(check, check_value);
		elm_box_pack_end(box, check);
		evas_object_show(check);
	}

	box2 = elm_box_add(win);
	elm_box_horizontal_set(box2, EINA_TRUE);
	elm_box_homogenous_set(box2, EINA_TRUE);
	evas_object_size_hint_align_set(box2, EVAS_HINT_FILL, 0);

	/* ok - yes - no - cancel */
	if (buttonflags & DIALOG_OK) {
		btn = elm_button_add(win);
		elm_button_label_set(btn, D_("Ok"));
		evas_object_data_set(btn, "type", GINT_TO_POINTER(DIALOG_OK));
		evas_object_size_hint_weight_set(btn, EVAS_HINT_EXPAND, 0);
		evas_object_size_hint_align_set(btn, EVAS_HINT_FILL, 0);
		evas_object_smart_callback_add(btn, "clicked",
					_inwin_dialog_cb, pack);
		evas_object_show(btn);
		elm_box_pack_end(box2, btn);
	}
	if (buttonflags & DIALOG_YES) {
		btn = elm_button_add(win);
		elm_button_label_set(btn, D_("Yes"));
		evas_object_data_set(btn, "type", GINT_TO_POINTER(DIALOG_YES));
		evas_object_size_hint_weight_set(btn, EVAS_HINT_EXPAND, 0);
		evas_object_size_hint_align_set(btn, EVAS_HINT_FILL, 0);
		evas_object_smart_callback_add(btn, "clicked",
					_inwin_dialog_cb, pack);
		evas_object_show(btn);
		elm_box_pack_end(box2, btn);
	}
	if (buttonflags & DIALOG_NO) {
		btn = elm_button_add(win);
		elm_button_label_set(btn, D_("No"));
		evas_object_data_set(btn, "type", GINT_TO_POINTER(DIALOG_NO));
		evas_object_size_hint_weight_set(btn, EVAS_HINT_EXPAND, 0);
		evas_object_size_hint_align_set(btn, EVAS_HINT_FILL, 0);
		evas_object_smart_callback_add(btn, "clicked",
					_inwin_dialog_cb, pack);
		evas_object_show(btn);
		elm_box_pack_end(box2, btn);
	}
	if (buttonflags & DIALOG_CANCEL) {
		btn = elm_button_add(win);
		elm_button_label_set(btn, D_("Cancel"));
		evas_object_data_set(btn, "type", GINT_TO_POINTER(DIALOG_CANCEL));
		evas_object_size_hint_weight_set(btn, EVAS_HINT_EXPAND, 0);
		evas_object_size_hint_align_set(btn, EVAS_HINT_FILL, 0);
		evas_object_smart_callback_add(btn, "clicked",
					_inwin_dialog_cb, pack);
		evas_object_show(btn);
		elm_box_pack_end(box2, btn);
	}
	evas_object_show(box2);
	elm_box_pack_end(box, box2);

	elm_win_resize_object_add(win, box);
 	evas_object_show(box);
	elm_win_inwin_content_set(pack->inwin, box);

	elm_win_inwin_activate(pack->inwin);
}


struct _inwin_list_pack {
	void (*callback)(const char *, void *);
	void *data;
	Evas_Object *inwin;
	Evas_Object *list;
};

static gboolean
_inwin_list_destruct(gpointer data)
{
	struct _inwin_list_pack *pack = (struct _inwin_list_pack *)data;
	evas_object_del(pack->inwin);
	free (pack);
	return FALSE;
}

static void
_inwin_list_selected_cb(void *data, Evas_Object *obj, void *event_info)
{
	(void) event_info;
	char *sel = NULL;
	struct _inwin_list_pack *pack = (struct _inwin_list_pack *)data;
	g_debug("Get the selected one");
	Elm_List_Item *it = elm_list_selected_item_get(obj);
	if (it) {
		// FIXME: either remove this strdup... or the const from the cb
		sel = strdup(elm_list_item_label_get(it));
		g_debug("Which is '%s'", sel);
	}
	if (pack->callback) {
		pack->callback(sel, pack->data);
	}
	g_timeout_add(0, _inwin_list_destruct, pack);
}

static void
_inwin_list_cancel_cb(void *data, Evas_Object *obj, void *event_info)
{
	(void) obj;
	(void) event_info;
	struct _inwin_list_pack *pack = (struct _inwin_list_pack *)data;
	g_debug("Cancelled selection");
	if (pack->callback) {
		pack->callback(NULL, pack->data);
	}
	g_timeout_add(0, _inwin_list_destruct, pack);
}

Evas_Object *
ui_utils_view_inwin_list(struct View *view, GList *list,
	void (*callback) (const char *, void *), void *userdata)
{
	Evas_Object *win, *btn, *box;
	GList *l;

	g_debug("ui_utils_view_inwin_list");
	struct _inwin_list_pack *pack = malloc(sizeof(struct _inwin_list_pack));
	pack->callback = callback;
	pack->data = userdata;
	win = ui_utils_view_window_get(view);
	pack->inwin = elm_win_inwin_add(win);

	box = elm_box_add(win);
// 	elm_box_homogenous_set(box, EINA_TRUE);
	evas_object_size_hint_weight_set(box, EVAS_HINT_EXPAND,
					 EVAS_HINT_EXPAND);

	pack->list = elm_list_add(win);
	elm_object_style_set(pack->list, "fieldlist");
 	elm_win_resize_object_add(win, pack->list);
	evas_object_size_hint_align_set(pack->list, EVAS_HINT_FILL,
					EVAS_HINT_FILL);
	evas_object_size_hint_weight_set(pack->list, EVAS_HINT_EXPAND,
					 EVAS_HINT_EXPAND);
// 	evas_object_size_hint_align_set(pack->list, 0.5, 0.5);
	elm_list_mode_set(pack->list, ELM_LIST_COMPRESS);
	for (l = g_list_first(list); l; l = g_list_next(l)) {
		g_debug("Adding item '%s' to list", (char *)l->data);
		elm_list_item_append(pack->list, strdup(l->data),
				     NULL, NULL, NULL, NULL);
	}
	g_list_free(list);
	evas_object_smart_callback_add(pack->list, "selected",
				       _inwin_list_selected_cb, pack);
	elm_list_go(pack->list);
	evas_object_show(pack->list);

 	elm_box_pack_end(box, pack->list);

	btn = elm_button_add(win);
	elm_button_label_set(btn, D_("Cancel"));
	evas_object_size_hint_weight_set(btn, EVAS_HINT_EXPAND, 0);
	evas_object_size_hint_align_set(btn, EVAS_HINT_FILL, 0);
	evas_object_smart_callback_add(btn, "clicked",
				       _inwin_list_cancel_cb, pack);
	evas_object_show(btn);
	elm_box_pack_end(box, btn);

	elm_win_resize_object_add(win, box);
 	evas_object_show(box);
	elm_win_inwin_content_set(pack->inwin, box);

	elm_win_inwin_activate(pack->inwin);

	return pack->inwin;
}

static void
_notify_button_close_cb(void *data, Evas_Object *obj, void *event_info)
{
	Evas_Object *notify = data;
	(void) obj;
	(void) event_info;
	evas_object_hide(notify);
}

Evas_Object *
ui_utils_notify(Evas_Object *parent, const char *label, int timeout)
{
	Evas_Object *notify, *bx, *bt, *lb;
	notify = elm_notify_add(parent);
	evas_object_size_hint_weight_set(notify, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	elm_notify_orient_set(notify, ELM_NOTIFY_ORIENT_CENTER);
	elm_notify_timeout_set(notify, timeout);

	bx = elm_box_add(parent);
	elm_notify_content_set(notify, bx);
	elm_box_horizontal_set(bx, 1);
	evas_object_show(bx);

	lb = elm_label_add(parent);
	elm_label_label_set(lb, label);
	elm_box_pack_end(bx, lb);
	evas_object_show(lb);

	bt = elm_button_add(parent);
	elm_button_label_set(bt, "Close");
	evas_object_smart_callback_add(bt, "clicked", _notify_button_close_cb, notify);
	elm_box_pack_end(bx, bt);
	evas_object_show(bt);

	return notify;
}


int
ui_utils_view_is_init(struct View *view)
{
	return (view->win) ? 1 : 0;
}

char *
ui_utils_entry_utf8_get(Evas_Object *entry)
{
	if (!entry)
		return NULL;
	/* this allocates a new char * */
	char *s = elm_entry_markup_to_utf8(elm_entry_entry_get(entry));
	if (s) {
		return g_strstrip(s);
	}
	else {
		/* Get around the buggy elm_entry_markup_to_utf8 */
		return strdup("");
	}
}

static void
error_message_ok_cb(void *data, Evas_Object *obj, void *event_info)
{
	(void)obj;
	(void)event_info;

	Evas_Object *inwin = (Evas_Object*)data;
	evas_object_del(inwin);
}

void
ui_utils_error_message_show(struct View *parent, const char *error_msg, const char *detail_msg)
{
	Evas_Object *win = ui_utils_view_window_get(parent);

	Evas_Object *inwin = elm_win_inwin_add(win);
	//evas_object_event_callback_add(inwin, EVAS_CALLBACK_DEL, error_message_del, NULL);

	Evas_Object *box = elm_box_add(win);
	elm_box_homogenous_set(box, EINA_FALSE);
	evas_object_size_hint_weight_set(box, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_size_hint_align_set(box, EVAS_HINT_FILL, EVAS_HINT_FILL);

	Evas_Object *heading = elm_label_add(win);
	//elm_label_line_wrap_set(heading, EINA_TRUE);
	elm_label_label_set(heading, error_msg);
	evas_object_size_hint_weight_set(heading, 1.0, 1.0);
	evas_object_size_hint_align_set(heading, 0.5, 0.5);
	evas_object_show(heading);
	elm_box_pack_end(box, heading);

	if (detail_msg) {
		Evas_Object *lb = elm_label_add(win);
		elm_label_line_wrap_set(lb, EINA_TRUE);
		elm_label_label_set(lb, detail_msg);
		evas_object_size_hint_weight_set(lb, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
		evas_object_size_hint_align_set(lb, EVAS_HINT_FILL, EVAS_HINT_FILL);
		evas_object_show(lb);

		Evas_Object *scroller = elm_scroller_add(win);
		evas_object_size_hint_weight_set(scroller, EVAS_HINT_EXPAND, 1.0);
		evas_object_size_hint_align_set(scroller, EVAS_HINT_FILL, EVAS_HINT_FILL);
		elm_scroller_content_set(scroller, lb);
		evas_object_show(scroller);

		elm_box_pack_end(box, scroller);
	}

	Evas_Object *bt = elm_button_add(win);
	elm_button_label_set(bt, D_("Ok"));
	evas_object_size_hint_weight_set(bt, EVAS_HINT_EXPAND, 0.0);
	evas_object_size_hint_align_set(bt, EVAS_HINT_FILL, 1.0);
	evas_object_smart_callback_add(bt, "clicked", error_message_ok_cb, inwin);

	evas_object_show(bt);
	elm_box_pack_end(box, bt);

	elm_win_inwin_content_set(inwin, box);

	elm_win_inwin_activate(inwin);
}

void
ui_utils_error_message_from_gerror_show(struct View *parent, const char *msg,
	const GError *err)
{
	if (err)
	{
		gchar *detail_msg = g_strdup_printf("(%d) %s",
			err->code, err->message);
		ui_utils_error_message_show(parent, msg, detail_msg);
		g_free(detail_msg);
	}
	else
		ui_utils_error_message_show(parent, msg, NULL);
}
