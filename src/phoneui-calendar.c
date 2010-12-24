/*
 *  Copyright (C) 2009, 2010
 *      Authors (alphabetical) :
 *		Tom "TAsn" Hacohen <tom@stosb.com>
 *		Klaus 'mrmoku' Kurzmann <mok@fluxnetz.de>
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
#include <phoneui/phoneui-utils-dates.h>
#include "phoneui-calendar.h"
#include "view/calendar-month-view.h"
#include "view/calendar-day-view.h"
#include "view/calendar-date-view.h"
#include "view/calendar-new-view.h"


void
phoneui_backend_calendar_month_show(const int month)
{
 	if (!calendar_month_view_is_init()) {
		if (calendar_month_view_init()) {
			return;
		}
	}
	calendar_month_view_show(month);
}

void
phoneui_backend_calendar_deinit()
{
	/*Also free open dates*/
	calendar_month_view_deinit();
}

static void
_date_get_cb(GError *error, GHashTable *content, gpointer data)
{
	char *path = (char *)data;
	if (error || !content) {
		g_warning("Failed aquiring data for date %s", path);
		free (path);
		return;
	}
	g_debug("Got data for date %s", path);
	if (calendar_date_view_init(path, content)) {
		g_warning("Init of view for date %s failed", path);
		return;
	}

	g_debug("Showing date %s", path);
	calendar_date_view_show(path);
}

void
phoneui_backend_calendar_date_show(const char *path)
{
	if (!calendar_date_view_is_init(path)) {
		g_debug("View for date %s is not yet inited...", path);
		phoneui_utils_date_get(path, _date_get_cb, strdup(path));
		return;
	}
	g_debug("View for date %s is inited - showing", path);
	calendar_date_view_show(path);
}

static void
_day_get_cb(GError *error, GHashTable *content, gpointer data)
{
	char *day = (char *)data;
	if (error || !content) {
		g_warning("Failed aquiring data for day %s", day);
		free (day);
		return;
	}
	g_debug("Got data for day %s", day);
	if (calendar_day_view_init(day, content)) {
		g_warning("Init of view for day %s failed", day);
		return;
	}

	g_debug("Showing day %s", day);
	calendar_day_view_show(day);
}

void
phoneui_backend_calendar_day_show(const char *day)
{
	if (!calendar_day_view_is_init(day)) {
		g_debug("View for day %s is not yet inited...", day);
		phoneui_utils_day_get(day, _day_get_cb, strdup(day));
		return;
	}
	g_debug("View for day %s is inited - showing", day);
	calendar_day_view_show(day);
}

void
phoneui_backend_calendar_date_new(GHashTable *options)
{
	struct View *view;
	g_debug("Initing new date view");
	view = calendar_new_view_init(options);
	if (!view) {
		return;
	}
	g_debug("Showing new date view");
	calendar_new_view_show(view);
}
