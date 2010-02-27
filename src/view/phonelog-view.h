#ifndef _PHONELOG_VIEW_H
#define _PHONELOG_VIEW_H

void phonelog_view_show();
void phonelog_view_hide();
int phonelog_view_init();
void phonelog_view_deinit();
int phonelog_view_is_init();
void phonelog_view_new_call(char *path);

#endif

