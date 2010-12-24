/*
 *  Copyright (C) 2009, 2010
 *      Authors (alphabetical) :
 *		Tom "TAsn" Hacohen <tom@stosb.com>
 *		Klaus 'mrmoku' Kurzmann <mok@fluxnetz.de>
 *		Martin Jansa <Martin.Jansa@gmail.com>
 *		Thomas Zimmermann <bugs@vdm-design.de>
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



#include <glib.h>
#include <dbus/dbus-glib.h>
#include <time.h>
#include <Elementary.h>
#include <phoneui/phoneui.h>
#include <phoneui/phoneui-info.h>
#include <phoneui/phoneui-utils-dates.h>

#include "phoneui-calendar.h"
#include "phoneui-shr.h"
#include "views.h"
#include "common-utils.h"
#include "ui-utils.h"
#include "calendar-month-view.h"

struct CalendarMonthViewData  {
	struct View view;
	char *path;
	Evas_Object *calendar, *hv, *bt_new, *bt_options;
};
static struct CalendarMonthViewData view;

static void _hover_bt_1(void *_data, Evas_Object * obj, void *event_info);
static void _delete_cb(struct View *data, Evas_Object *obj, void *event_info);
static void _new_clicked(void *_data, Evas_Object * obj, void *event_info);
static void _process_dates(GError* error, GHashTable** dates, int count,
			   gpointer data);
static void _process_date(gpointer _date, gpointer _data);

int
calendar_month_view_init()
{
	Evas_Object *win, *box, *box2, *sc;
	int ret;

	g_debug("Initing calendar month view");
	ret = ui_utils_view_init(VIEW_PTR(view), ELM_WIN_BASIC, D_("Calendar"),
				 NULL, NULL, NULL);
	if (ret) {
		g_critical("Failed to init the calendar month view");
		return ret;
	}

	win = ui_utils_view_window_get(VIEW_PTR(view));
	ui_utils_view_delete_callback_set(VIEW_PTR(view), _delete_cb);
	ui_utils_view_layout_set(VIEW_PTR(view), phoneui_theme,
				 "phoneui/calendar/month");
	elm_theme_extension_add(NULL, phoneui_theme);

	sc = elm_scroller_add(win);
	ui_utils_view_swallow(VIEW_PTR(view), "main_view", sc);
	evas_object_show(sc);
	
	box = elm_box_add(win);
	elm_box_horizontal_set(box, 0);
	elm_box_homogenous_set(box, 0);
	evas_object_size_hint_align_set(box, EVAS_HINT_FILL, EVAS_HINT_FILL);
	evas_object_size_hint_weight_set(box, EVAS_HINT_EXPAND,
					 EVAS_HINT_EXPAND);
	evas_object_show(box);
	elm_scroller_content_set(sc, box);

	view.calendar = elm_calendar_add(win);
	evas_object_show(view.calendar);
	elm_box_pack_end(view.calendar, box);
/*
	evas_object_smart_callback_add(view.calendar, "changed", _changed_cb,
				       NULL);
*/
	box2 = elm_box_add(win);
	elm_box_horizontal_set(box2, 1);
	elm_box_homogenous_set(box2, 1);
	evas_object_show(box2);
	elm_box_pack_end(box, box2);
	evas_object_size_hint_align_set(box2, EVAS_HINT_FILL, EVAS_HINT_FILL);
	evas_object_size_hint_weight_set(box2, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);

	view.bt_new = elm_button_add(win);
	elm_button_label_set(view.bt_new, D_("New"));
	evas_object_smart_callback_add(view.bt_new, "clicked", _new_clicked,
				       NULL);
	evas_object_size_hint_align_set(view.bt_new, EVAS_HINT_FILL,
					EVAS_HINT_FILL);
	evas_object_size_hint_weight_set(view.bt_new, EVAS_HINT_EXPAND,
					 EVAS_HINT_EXPAND);
	elm_box_pack_start(box2, view.bt_new);
	/*ui_utils_view_swallow(VIEW_PTR(view), "button_new", view.bt_new);*/
	evas_object_show(view.bt_new);

	// Options button with hover
	view.bt_options = elm_button_add(win);
	elm_button_label_set(view.bt_options, D_("Options"));
	evas_object_smart_callback_add(view.bt_options, "clicked", _hover_bt_1,
				       NULL);
	evas_object_size_hint_align_set(view.bt_options, EVAS_HINT_FILL,
					EVAS_HINT_FILL);
	evas_object_size_hint_weight_set(view.bt_options, EVAS_HINT_EXPAND,
					 EVAS_HINT_EXPAND);
	/*ui_utils_view_swallow(VIEW_PTR(view), "button_options",
			      view.bt_options);*/
	elm_box_pack_end(box2, view.bt_options);
	evas_object_show(view.bt_options);
/*
	view.hv = elm_hover_add(win);

	elm_hover_parent_set(view.hv, win);
	elm_hover_target_set(view.hv, obj);

	box = elm_box_add(win);
	elm_box_horizontal_set(box, 0);
	elm_box_homogenous_set(box, 1);
	evas_object_show(box);

	obj = elm_button_add(win);
	elm_button_label_set(obj, D_("Answer"));
	evas_object_size_hint_min_set(obj, 130, 80);
	evas_object_smart_callback_add(obj, "clicked", _answer_clicked, NULL);
	evas_object_show(obj);
	elm_box_pack_end(box, obj);

	obj = elm_button_add(win);
	elm_button_label_set(obj, D_("Forward"));
	evas_object_size_hint_min_set(obj, 130, 80);
	evas_object_smart_callback_add(obj, "clicked", _forward_clicked, NULL);
	evas_object_show(obj);
	elm_box_pack_end(box, obj);

	obj = elm_button_add(win);
	elm_button_label_set(obj, D_("Delete"));
	evas_object_size_hint_min_set(obj, 130, 80);
	evas_object_smart_callback_add(obj, "clicked", _delete_clicked, NULL);
	evas_object_show(obj);
	elm_box_pack_end(box, obj);

	elm_hover_content_set(view.hv, "top", box);
*/
	return 0;
}

int
calendar_month_view_is_init()
{
	return ui_utils_view_is_init(VIEW_PTR(view));
}

void
calendar_month_view_deinit()
{
	ui_utils_view_deinit(VIEW_PTR(view));
}

void
calendar_month_view_show(const int month)
{
	time_t tts;
	if (month > 0)
		tts = (long int) month;
	else
		tts = time(NULL);

	elm_calendar_selected_time_set(view.calendar, localtime(&tts));

	struct tm *begin = localtime(&tts);
	begin->tm_sec = 0;
	begin->tm_min = 0;
	begin->tm_hour = 0;
	begin->tm_mday = 1;
	
	time_t begin_t = mktime(begin);

	struct tm *end = localtime(&tts);
	end->tm_sec = 60;
	end->tm_min = 60;
	end->tm_hour = 12;
	end->tm_mday = 31;

	time_t end_t = mktime(end);

	g_debug("Filling calendar with dates with timestamp from %ld to %ld", (long int)begin_t, (long int)end_t);
	phoneui_utils_dates_get_full("Begin", TRUE, begin_t, end_t, _process_dates, NULL);

	evas_object_hide(view.hv);
	ui_utils_view_show(VIEW_PTR(view));
}

void
calendar_month_view_hide()
{
	ui_utils_view_hide(VIEW_PTR(view));
}

/* Callbacks */

static void
_hover_bt_1(void *_data, Evas_Object * obj, void *event_info)
{
	(void) _data;
	(void) obj;
	(void) event_info;
	/*evas_object_show(view.hv);*/
}

static void
_new_clicked(void *_data, Evas_Object * obj, void *event_info)
{
	(void) _data;
	(void) obj;
	(void) event_info;
	phoneui_backend_calendar_date_new(NULL);
}

static void
_delete_cb(struct View *data, Evas_Object *obj, void *event_info)
{
	(void)data;
	(void)obj;
	(void)event_info;
	g_debug("_delete_cb");
	calendar_month_view_hide();
}

static void
_process_dates(GError* error, GHashTable** dates, int count, gpointer data)
{
	int i;

	g_debug("got %d dates", count);

	if (error) {
		ui_utils_notify(ui_utils_view_window_get(VIEW_PTR(view)),
				D_("Error while retrieving dates"), 10);
		g_warning("Error retrieving dates: (%d) %s",
			  error->code, error->message);
		return;
	}
	if (!dates) {
		ui_utils_notify(ui_utils_view_window_get(VIEW_PTR(view)),
				D_("There are no dates"), 5);
		g_debug("No dates to load");
		return;
	}

	for (i = 0; i < count; i++) {
		g_debug("processing date %d", i);
		_process_date(dates[i], data);
	}

	elm_calendar_marks_draw(view.calendar);
}


static void
_process_date(gpointer _date, gpointer _data)
{
	(void)_data;
	GHashTable *date;
	GValue *gval_tmp;
	time_t begin;

	if (!_date) {
		return;
	}
	date = (GHashTable *)_date;

	gval_tmp = g_hash_table_lookup(date, "Path");
	if (!gval_tmp) {
		g_critical("Date without Path?!?");
		return;
	}
/*
	path =  g_value_get_char(gval_tmp);
*/
	gval_tmp = g_hash_table_lookup(date, "Begin");
	if (!gval_tmp) {
		g_critical("Date without Begin?!?");
		return;
	}
	begin = (time_t) g_value_get_long(gval_tmp);

	gval_tmp = g_hash_table_lookup(date, "End");
	if (!gval_tmp) {
		g_critical("Date without End?!?");
		return;
	}
/*
	end = g_value_get_long(gval_tmp);
*/
	elm_calendar_mark_add(view.calendar, "checked", localtime(&begin), ELM_CALENDAR_UNIQUE);
}
