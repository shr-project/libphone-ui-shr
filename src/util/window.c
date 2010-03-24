
#include "phoneui-shr.h"
#include "window.h"
#include <stdlib.h>
#include <Ecore_Evas.h>
#include <Ecore.h>
#include <Edje.h>
#include <string.h>
#include <glib.h>

// TODO: Remove this:
#include "phoneui-init.h"


static void
  _window_delete_callback(void *data, Evas_Object * win, void *event_info);


struct Window *
window_new(char *title)
{
	g_debug("window_new");
	struct Window *win;
	win = calloc(1, sizeof(struct Window));
	win->title = strdup(title);

	return (win);
}

void
window_init(struct Window *win)
{
	g_debug("window_init(win=%d)", (int) win);

	if (phoneui_theme)
		elm_theme_overlay_add(phoneui_theme);

	// Window
	win->win = elm_win_add(NULL, "main", ELM_WIN_BASIC);
	if (!win->win) {
		g_critical("Wasn't able to create a window for idle_screen");
		return;
	}

	elm_win_title_set(win->win, win->title);
	elm_win_autodel_set(win->win, 1);	// Disable it?
	evas_object_smart_callback_add(win->win, "delete,request",
				       _window_delete_callback, win);

	// Background
	win->bg = elm_bg_add(win->win);
	evas_object_size_hint_weight_set(win->bg, 1.0, 1.0);
	elm_win_resize_object_add(win->win, win->bg);
	evas_object_show(win->bg);

	// Layout
	win->layout = elm_layout_add(win->win);
	elm_win_resize_object_add(win->win, win->layout);
	evas_object_show(win->layout);

	evas_object_resize(win->win, 480, 600);
}

void
window_show(struct Window *win)
{
	if (win) {
		g_debug("window_show(win=%d)", (int) win);
		evas_object_show(win->win);
		elm_win_activate(win->win);
	}
	else {
		g_critical("Window is NULL (%s:%d)", __FUNCTION__, __LINE__);
	}
}

void
window_layout_set(struct Window *win, const char *file, const char *part)
{
	if (!win) {
		g_critical("Window is NULL (%s:%d)", __FUNCTION__, __LINE__);
		return;
	}
	g_debug("setting layout from file '%s' (%s)", file, part);
	elm_layout_file_set(win->layout, file, part);
}

Evas_Object *
window_layout_get(struct Window *win)
{
	if (!win) {
		g_critical("Window is NULL (%s:%d)", __FUNCTION__, __LINE__);
		return NULL;
	}
	return elm_layout_edje_get(win->layout);
}

Evas_Object *
window_evas_object_get(struct Window * win)
{
	if (!win) {
		g_critical("Window is NULL (%s:%d)", __FUNCTION__, __LINE__);
		return NULL;
	}
	return win->win;
}

void
window_delete_callback_set(struct Window *win,
			   void (*cb) (void *data, Evas_Object * win,
				       void *event_info))
{
	if (!win) {
		g_critical("Window is NULL (%s:%d)", __FUNCTION__, __LINE__);
		return;
	}
	if (!cb) {
		g_warning("Tried to set cb to NULL (%s:%d)", __FUNCTION__, __LINE__);
		return;
	}
	evas_object_smart_callback_add(win->win, "delete,request", cb, win);
}

void
window_text_set(struct Window *win, const char *key, const char *value)
{
	if (!win) {
		g_critical("Window is NULL (%s:%d)", __FUNCTION__, __LINE__);
		return;
	}
	edje_object_part_text_set(elm_layout_edje_get(win->layout), key, value);
}

void
window_swallow(struct Window *win, const char *key, Evas_Object * object)
{
	if (!win) {
		g_critical("Window is NULL (%s:%d)", __FUNCTION__, __LINE__);
		return;
	}
	//edje_object_part_swallow(elm_layout_edje_get(win->layout), key, object);
	elm_layout_content_set(win->layout, key, object);
}

void
window_unswallow(struct Window *win, Evas_Object * object)
{
	if (!win) {
		g_critical("Window is NULL (%s:%d)", __FUNCTION__, __LINE__);
		return;
	}
	edje_object_part_unswallow(elm_layout_edje_get(win->layout), object);
}

void
window_view_show(struct Window *win, void *options,
		 void *(*show_cb) (struct Window * win, void *options),
		 void (*hide_cb) (void *data), void (*exit_cb)())
{
	g_debug("window_view_show()");
	if (!win) {
		g_critical("Window is NULL (%s:%d)", __FUNCTION__, __LINE__);
		return;
	}
	if (!show_cb) {
		g_warning("Tried to set cb to NULL (%s:%d)", __FUNCTION__, __LINE__);
		return;
	}

	// Clear old view
	window_view_hide(win, NULL);

	if (show_cb != NULL)
		win->view_data = show_cb(win, options);

	win->view_hide_cb = hide_cb;
	win->exit_cb = exit_cb;
}

void
window_view_hide(struct Window *win, void *options)
{
	(void) options;
	g_debug("window_view_hide()");
	if (!win) {
		g_critical("Window is NULL (%s:%d)", __FUNCTION__, __LINE__);
		return;
	}

	// Hide last frame
	window_frame_hide(win, win->view_data);

	if (win->view_hide_cb != NULL) {
		win->view_hide_cb(win->view_data);
		win->view_hide_cb = NULL;
	}

	evas_object_hide(win->win);

	win->view_data = NULL;
}



void
window_frame_show(struct Window *win, void *data, void (*show_cb) (void *data),
		  void (*hide_cb) (void *data))
{
	if (!win) {
		g_critical("Window is NULL (%s:%d)", __FUNCTION__, __LINE__);
		return;
	}

	if (win->frame_hide_cb != NULL)
		win->frame_hide_cb(data);
	else
		g_debug("No frame to hide");

	// hide_cb could be NULL!
	win->frame_hide_cb = hide_cb;

	show_cb(data);
}

void
window_frame_hide(struct Window *win, void *data)
{
	if (!win) {
		g_critical("Window is NULL (%s:%d)", __FUNCTION__, __LINE__);
		return;
	}

	if (win->frame_hide_cb != NULL) {
		win->frame_hide_cb(data);
		win->frame_hide_cb = NULL;
	}
}

void
window_kbd_show(struct Window *win, KeyboardMode mode)
{
	ecore_x_e_virtual_keyboard_state_set(ecore_evas_software_x11_window_get
					     (ecore_evas_ecore_evas_get
					      (evas_object_evas_get(win->win))),
					     mode);
}

void
window_kbd_hide(struct Window *win)
{
	ecore_x_e_virtual_keyboard_state_set(ecore_evas_software_x11_window_get
					     (ecore_evas_ecore_evas_get
					      (evas_object_evas_get(win->win))),
					     KEYBOARD_OFF);
}

void
window_destroy(struct Window *win, void *options)
{
	g_debug("destroying window (win=%d)", (int) win);
	if (!win) {
		g_critical("Window is NULL (%s:%d)", __FUNCTION__, __LINE__);
		return;
	}
	window_view_hide(win, options);

	evas_object_del(win->layout);
	evas_object_del(win->bg);
	evas_object_del(win->win);

	if (win->exit_cb)
		win->exit_cb();
	free(win);
}


Evas_Object *
window_inwin_dialog(struct Window *win, const char *label, GList * buttons,
		    void *data)
{
	Evas_Object *inwin = elm_win_inwin_add(window_evas_object_get(win));

	Evas_Object *bx = elm_box_add(window_evas_object_get(win));
	elm_box_homogenous_set(bx, 1);

	Evas_Object *e = elm_label_add(window_evas_object_get(win));
	elm_label_label_set(e, label);
	evas_object_show(e);
	elm_box_pack_end(bx, e);

	Evas_Object *bx2 = elm_box_add(window_evas_object_get(win));
	elm_box_horizontal_set(bx2, 1);
	elm_box_homogenous_set(bx2, 1);

	for (buttons = g_list_first(buttons); buttons;
	     buttons = g_list_next(buttons)) {
		e = elm_button_add(window_evas_object_get(win));
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



static void
_window_delete_callback(void *data, Evas_Object * win, void *event_info)
{
	(void) win;
	(void) event_info;
	g_debug("_window_delete_callback");
	window_destroy(data, NULL);
}
