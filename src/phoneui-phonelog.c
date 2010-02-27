#include "phoneui-phonelog.h"
#include "phonelog-view.h"

void
phoneui_backend_phone_log_show()
{
	if (!phonelog_view_is_init()) {
		if (phonelog_view_init()) {
			return;
		}
	}
	phonelog_view_show();
}

void
phoneui_backend_phone_log_hide()
{
	phonelog_view_hide();
}

void
phoneui_backend_phone_log_new_call(char *path)
{
	phonelog_view_new_call(path);
}

