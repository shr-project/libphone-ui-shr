#include "phoneui-phone-log.h"
#include "phone-log-view.h"

void
phoneui_backend_phone_log_show()
{
	if (!phone_log_view_is_init()) {
		if (phone_log_view_init()) {
			return;
		}
	}
	phone_log_view_show();
}

void
phoneui_backend_phone_log_hide()
{
	phone_log_view_hide();
}
