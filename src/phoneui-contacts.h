#ifndef _PHONEGUI_CONTACTS_H
#define _PHONEGUI_CONTACTS_H

void phoneui_backend_contacts_show();
void phoneui_backend_contacts_refresh();
void phoneui_backend_contacts_contact_show(const char *path);
void phoneui_backend_contacts_contact_new(const char *name, const char *number);
void phoneui_backend_contacts_contact_edit(const char *path);

#endif
