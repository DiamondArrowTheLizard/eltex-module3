#ifndef ARRAY_CONTACTS_H
#define ARRAY_CONTACTS_H

#include "personal_info.h"
#include <stddef.h>

#define ARRAY_CONTACTS_OK 1
#define ARRAY_CONTACTS_ERROR 0
#define ARRAY_CONTACTS_CAPACITY 50

typedef struct array_contacts
{
    personal_info* contacts[ARRAY_CONTACTS_CAPACITY];
    size_t size;
} array_contacts;

array_contacts* array_contacts_init(void);
void array_contacts_destroy(array_contacts* array);
int array_contacts_append(array_contacts* array, personal_info* contact);
int array_contacts_remove(array_contacts* array, size_t index);
void array_contacts_print_all(const array_contacts* array);
int array_contacts_print_at_index(const array_contacts* array, size_t index);

void array_contacts_load(array_contacts* array, const char* filename);
void array_contacts_save(const array_contacts* array, const char* filename);

#endif // !ARRAY_CONTACTS_H
