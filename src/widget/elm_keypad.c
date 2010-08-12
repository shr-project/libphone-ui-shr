/*
 *  Copyright (C) 2009, 2010
 *      Authors (alphabetical) :
 *		Tom "TAsn" Hacohen <tom@stosb.com>
 *		Klaus 'mrmoku' Kurzmann <mok@fluxnetz.de>
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

#include <Elementary.h>
#include <glib-2.0/glib.h>

#include "phoneui-shr.h"
#include "views.h"


/* HACKS FROM elm_priv.h that should be removed */
#if 1
void         elm_widget_data_set(Evas_Object *obj, void *data);
void        *elm_widget_data_get(const Evas_Object *obj);
void         elm_widget_del_hook_set(Evas_Object *obj, void (*func) (Evas_Object *obj));
void         elm_widget_resize_object_set(Evas_Object *obj, Evas_Object *sobj);
Evas_Object *elm_widget_add(Evas *evas);
#endif

typedef struct _Widget_Data Widget_Data;

struct _Widget_Data {
	Evas_Object *widget, *keypad;
	Ecore_Timer *plus_timer;
};

static void  _del_hook(Evas_Object * obj);
static void _sizing_eval(Evas_Object * obj);
static void _signal_clicked(void *data, Evas_Object * o, const char *emission,
		const char *source);
static void _zero_mouse_down(void *data, Evas_Object * o, const char *emission,
		 const char *source);
static void _zero_mouse_up(void *data, Evas_Object * o, const char *emission,
	       const char *source);
static Eina_Bool _plus_trigered(void *data);

static void
_del_hook(Evas_Object * obj)
{
	Widget_Data *wd = elm_widget_data_get(obj);
	evas_object_del(wd->keypad);
	free(wd);
}

static void
_sizing_eval(Evas_Object * obj)
{
	Widget_Data *wd = elm_widget_data_get(obj);
	Evas_Coord minw = -1, minh = -1, maxw = -1, maxh = -1;

	edje_object_size_min_calc(wd->keypad, &minw, &minh);
	evas_object_size_hint_min_set(obj, minw, minh);
	evas_object_size_hint_max_set(obj, maxw, maxh);
}

static void
_signal_clicked(void *data, Evas_Object * o, const char *emission,
		const char *source)
{
	(void) o;
	(void) source;
	Widget_Data *wd = elm_widget_data_get(data);
	/* FIXME: may leak */
	evas_object_smart_callback_call(wd->widget, "clicked", strdup(emission));
}

static void
_zero_mouse_down(void *data, Evas_Object * o, const char *emission,
		 const char *source)
{
	(void) o;
	(void) emission;
	(void) source;
	Widget_Data *wd = elm_widget_data_get(data);
	if (wd->plus_timer == NULL)
		wd->plus_timer = ecore_timer_add(0.5, _plus_trigered, data);
}

static void
_zero_mouse_up(void *data, Evas_Object * o, const char *emission,
	       const char *source)
{
	(void) o;
	(void) emission;
	(void) source;
	Widget_Data *wd = elm_widget_data_get(data);

	if (wd->plus_timer != NULL) {
		ecore_timer_del(wd->plus_timer);
		wd->plus_timer = NULL;
		evas_object_smart_callback_call(wd->widget, "clicked", "0");
	}
}

static Eina_Bool
_plus_trigered(void *data)
{
	Widget_Data *wd = elm_widget_data_get(data);
	wd->plus_timer = NULL;
	evas_object_smart_callback_call(wd->widget, "clicked", "+");
	return (0);
}

EAPI Evas_Object *
elm_keypad_add(Evas_Object * parent)
{
	// Evas_Object *obj; Instead I'm using the wd->widget variable
	Evas *e;
	Widget_Data *wd;

	wd = calloc(1, sizeof(Widget_Data));
	e = evas_object_evas_get(parent);
	wd->widget = elm_widget_add(e);
	elm_widget_data_set(wd->widget, wd);
	elm_widget_del_hook_set(wd->widget, _del_hook);

	wd->keypad = edje_object_add(e);
	edje_object_file_set(wd->keypad, phoneui_theme, "phoneui/keypad");
	edje_object_signal_callback_add(wd->keypad, "*", "input",
					_signal_clicked, wd->widget);
	edje_object_signal_callback_add(wd->keypad, "0", "mouse_up",
					_zero_mouse_up, wd->widget);
	edje_object_signal_callback_add(wd->keypad, "0", "mouse_down",
					_zero_mouse_down, wd->widget);

	edje_object_part_text_set(wd->keypad, "text2_desc", D_("ABC"));
	edje_object_part_text_set(wd->keypad, "text3_desc", D_("DEF"));
	edje_object_part_text_set(wd->keypad, "text4_desc", D_("GHI"));
	edje_object_part_text_set(wd->keypad, "text5_desc", D_("JKL"));
	edje_object_part_text_set(wd->keypad, "text6_desc", D_("MNO"));
	edje_object_part_text_set(wd->keypad, "text7_desc", D_("PQRS"));
	edje_object_part_text_set(wd->keypad, "text8_desc", D_("TUV"));
	edje_object_part_text_set(wd->keypad, "text9_desc", D_("XYZ"));

	elm_widget_resize_object_set(wd->widget, wd->keypad);
	//evas_object_smart_callback_add(wd->widget, "sub-object-del", _sub_del, wd->widget);

	_sizing_eval(wd->widget);

	return wd->widget;
}
