#ifndef _QUICK_SETTINGS_VIEW_H
#define _QUICK_SETTINGS_VIEW_H

/*
 * vim:ts=4
 * 
 * Copyright © 2009 Rui Miguel Silva Seabra <rms@1407.org>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the 
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

void quick_settings_view_show();
void quick_settings_view_hide();
int quick_settings_view_init();
void quick_settings_view_deinit();
int quick_settings_view_is_init();

#endif
