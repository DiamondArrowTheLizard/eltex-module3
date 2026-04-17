#include "array_contacts.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>

array_contacts* array_contacts_init(void)
{
    array_contacts* array = (array_contacts*)malloc(sizeof(array_contacts));
    if (!array) return NULL;
    array->size = 0;
    for (int i = 0; i < ARRAY_CONTACTS_CAPACITY; ++i) {
        array->contacts[i] = NULL;
    }
    return array;
}

void array_contacts_destroy(array_contacts* array)
{
    if (!array) return;
    for (size_t i = 0; i < array->size; ++i) {
        personal_info_destroy(array->contacts[i]);
    }
    free(array);
}

int array_contacts_append(array_contacts* array, personal_info* contact)
{
    if (!array || !contact) return ARRAY_CONTACTS_ERROR;
    if (array->size >= ARRAY_CONTACTS_CAPACITY) return ARRAY_CONTACTS_ERROR;
    array->contacts[array->size] = contact;
    array->size++;
    return ARRAY_CONTACTS_OK;
}

int array_contacts_remove(array_contacts* array, size_t index)
{
    if (!array || index >= array->size) return ARRAY_CONTACTS_ERROR;
    personal_info_destroy(array->contacts[index]);
    for (size_t i = index; i < array->size - 1; ++i) {
        array->contacts[i] = array->contacts[i + 1];
    }
    array->size--;
    return ARRAY_CONTACTS_OK;
}

void array_contacts_print_all(const array_contacts* array)
{
    if (!array) return;
    for (size_t i = 0; i < array->size; ++i) {
        array_contacts_print_at_index(array, i);
    }
}

int array_contacts_print_at_index(const array_contacts* array, size_t index)
{
    if (!array || index >= array->size) return ARRAY_CONTACTS_ERROR;
    printf("\nContact %ld: \n", index);
    personal_info_print_all(array->contacts[index]);
    return ARRAY_CONTACTS_OK;
}

void array_contacts_load(array_contacts* array, const char* filename)
{
    if (!array) return;
    int fd = open(filename, O_RDONLY | O_CREAT, 0644);
    if (fd < 0) return;

    personal_info temp;
    ssize_t bytes;
    while (array->size < ARRAY_CONTACTS_CAPACITY) {
        bytes = read(fd, &temp, sizeof(personal_info));
        if (bytes != sizeof(personal_info)) break;

        personal_info* info = personal_info_create(temp.name, temp.surname);
        if (!info) break;
        personal_info_new_patronymic(info, temp.patronymic);
        personal_info_new_workplace(info, temp.workplace);
        personal_info_new_position(info, temp.position);
        personal_info_new_phone_numbers(info, temp.phone_numbers, temp.phone_count);
        personal_info_new_links(info, temp.links, temp.link_count);
        array_contacts_append(array, info);
    }
    close(fd);
}

void array_contacts_save(const array_contacts* array, const char* filename)
{
    if (!array) return;
    int fd = open(filename, O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (fd < 0) return;

    for (size_t i = 0; i < array->size; ++i) {
        personal_info* info = array->contacts[i];
        if (!info) continue;
        ssize_t written = write(fd, info, sizeof(personal_info));
        if (written != sizeof(personal_info)) break;
    }
    close(fd);
}
