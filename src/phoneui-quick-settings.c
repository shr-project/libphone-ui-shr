#include "phoneui-quick-settings.h"

#include "view/quick-settings-view.h"

void
phoneui_backend_quick_settings_show()
{
        if (!quick_settings_view_is_init()) {
                if (quick_settings_view_init()) {
                        return;
                }
        }
        quick_settings_view_show();
}

void
phoneui_backend_quick_settings_deinit()
{
	quick_settings_view_deinit();
}

void
phoneui_backend_quick_settings_hide()
{
        quick_settings_view_hide();
}
