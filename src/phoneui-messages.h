#ifndef _PHONEGUI_MESSAGES_H
#define _PHONEGUI_MESSAGES_H
#include <glib.h>

void phoneui_backend_messages_show();
void phoneui_backend_messages_message_show(const char *path);
void phoneui_backend_messages_message_new(GHashTable *options);

#endif
