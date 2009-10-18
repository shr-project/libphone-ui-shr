#ifndef _PHONEGUI_CONTACTS_H
#define _PHONEGUI_CONTACTS_H

void phoneui_backend_contacts_show();
void phoneui_backend_contacts_refresh();
void phoneui_backend_contacts_contact_show(const char *path);
void phoneui_backend_contacts_new_show(const char *name, const char *number);

#endif
