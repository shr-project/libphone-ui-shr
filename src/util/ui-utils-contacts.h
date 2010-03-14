#ifndef _UI_UTILS_CONTACTS_H
#define _UI_UTILS_CONTACTS_H

void ui_utils_contacts_field_select(struct View *view, void (*callback)(const char *, void *), void *data);
void ui_utils_contacts_contact_number_select(const char *path, void (*cb)(char *, void *), void *data);

#endif