
#include "window.h"
#include <stdlib.h>
#include <Ecore_Evas.h>
#include <Ecore.h>
#include <Edje.h>
#include <string.h>
#include <assert.h>
#include <glib.h>

// TODO: Remove this:
#include "phonegui-init.h"


static int window_counter = 0;

static void
  _window_delete_callback(void *data, Evas_Object * win, void *event_info);


struct Window *
window_new(char *title)
{
	struct Window *win;
	win = g_slice_alloc0(sizeof(struct Window));
	win->title = strdup(title);

	window_counter++;
	return win;
}

void
window_init(struct Window *win)
{
	g_debug("window_init(win=%d)", win);

	// Window
	win->win = elm_win_add(NULL, "main", ELM_WIN_BASIC);
	assert(win->win != NULL);
	elm_win_title_set(win->win, win->title);
	elm_win_autodel_set(win->win, 1);	// Disable it?
	evas_object_smart_callback_add(win->win, "delete-request",
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
	assert(win != NULL);
	evas_object_show(win->win);
}

void
window_layout_set(struct Window *win, const char *file, const char *part)
{
	assert(win != NULL);
	elm_layout_file_set(win->layout, file, part);
}

Evas_Object *
window_layout_get(struct Window *win)
{
	assert(win != NULL);
	return elm_layout_edje_get(win->layout);
}

Evas_Object *
window_evas_object_get(struct Window * win)
{
	assert(win != NULL);
	return win->win;
}

void
window_delete_callback_set(struct Window *win,
			   void (*cb) (void *data, Evas_Object * win,
				       void *event_info))
{
	assert(win != NULL);
	assert(cb != NULL);
	evas_object_smart_callback_add(win->win, "delete-request", cb, win);
}

void
window_text_set(struct Window *win, const char *key, const char *value)
{
	assert(win != NULL);
	edje_object_part_text_set(elm_layout_edje_get(win->layout), key, value);
}

void
window_swallow(struct Window *win, const char *key, Evas_Object * object)
{
	assert(win != NULL);
	//edje_object_part_swallow(elm_layout_edje_get(win->layout), key, object);
	elm_layout_content_set(win->layout, key, object);
}

void
window_unswallow(struct Window *win, Evas_Object * object)
{
	assert(win != NULL);
	edje_object_part_unswallow(elm_layout_edje_get(win->layout), object);
}

void
window_view_show(struct Window *win, void *options,
		 void *(*show_cb) (struct Window * win, void *options),
		 void (*hide_cb) (void *data))
{
	g_debug("window_view_show()");
	assert(win != NULL);
	assert(show_cb != NULL);

	// Clear old view
	window_view_hide(win, NULL);

	if (show_cb != NULL)
		win->view_data = show_cb(win, options);

	win->view_hide_cb = hide_cb;
}

void
window_view_hide(struct Window *win, void *options)
{
	g_debug("window_view_hide()");
	assert(win != NULL);

	// Hide last frame
	window_frame_hide(win, win->view_data);

	if (win->view_hide_cb != NULL) {
		win->view_hide_cb(win->view_data);
		win->view_hide_cb = NULL;
	}

	win->view_data = NULL;
}



void
window_frame_show(struct Window *win, void *data, void (*show_cb) (void *data),
		  void (*hide_cb) (void *data))
{
	assert(show_cb != NULL);

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
	assert(win != NULL);

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
	assert(win != NULL);
	window_view_hide(win, options);

	evas_object_hide(win->win);
	evas_object_del(win->layout);
	evas_object_del(win->bg);
	evas_object_del(win->win);
	g_slice_free1(sizeof(struct Window), win);

	window_counter--;
	if (window_counter == 0) {
		if (phonegui_exit_cb != NULL) {
			g_debug("calling exit_cb()");
			phonegui_exit_cb();
		}
	}
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
				     ((struct InwinButton *) buttons->
				      data)->label);
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
	window_destroy(data, NULL);
}
