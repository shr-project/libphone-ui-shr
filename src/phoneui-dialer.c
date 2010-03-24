#include "phoneui-dialer.h"
#include "dialer-view.h"

void
phoneui_backend_dialer_show()
{
	if (!dialer_view_is_init()) {
		if (dialer_view_init()) {
			return;
		}
	}
	dialer_view_show();
}

void
phoneui_backend_dialer_deinit()
{
	dialer_view_deinit();
}

