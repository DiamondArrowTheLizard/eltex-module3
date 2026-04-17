#include "personal_info.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

personal_info* personal_info_create(const char* name, const char* surname)
{
    personal_info* info = (personal_info*)malloc(sizeof(personal_info));
    if (!info) return NULL;

    info->name[0] = '\0';
    info->surname[0] = '\0';
    info->patronymic[0] = '\0';
    info->workplace[0] = '\0';
    info->position[0] = '\0';
    info->phone_count = 0;
    info->link_count = 0;
    for (int i = 0; i < INFO_MAX_COUNT; ++i) {
        info->phone_numbers[i][0] = '\0';
        info->links[i][0] = '\0';
    }

    if (name) strncpy(info->name, name, INFO_LEN - 1);
    if (surname) strncpy(info->surname, surname, INFO_LEN - 1);
    info->name[INFO_LEN - 1] = '\0';
    info->surname[INFO_LEN - 1] = '\0';
    return info;
}

void personal_info_destroy(personal_info* info)
{
    free(info);
}

void personal_info_new_name(personal_info* info, const char* name)
{
    if (info && name) {
        strncpy(info->name, name, INFO_LEN - 1);
        info->name[INFO_LEN - 1] = '\0';
    }
}

void personal_info_new_surname(personal_info* info, const char* surname)
{
    if (info && surname) {
        strncpy(info->surname, surname, INFO_LEN - 1);
        info->surname[INFO_LEN - 1] = '\0';
    }
}

void personal_info_new_patronymic(personal_info* info, const char* patronymic)
{
    if (info && patronymic) {
        strncpy(info->patronymic, patronymic, INFO_LEN - 1);
        info->patronymic[INFO_LEN - 1] = '\0';
    } else if (info) {
        info->patronymic[0] = '\0';
    }
}

void personal_info_new_workplace(personal_info* info, const char* workplace)
{
    if (info && workplace) {
        strncpy(info->workplace, workplace, INFO_LEN - 1);
        info->workplace[INFO_LEN - 1] = '\0';
    } else if (info) {
        info->workplace[0] = '\0';
    }
}

void personal_info_new_position(personal_info* info, const char* position)
{
    if (info && position) {
        strncpy(info->position, position, INFO_LEN - 1);
        info->position[INFO_LEN - 1] = '\0';
    } else if (info) {
        info->position[0] = '\0';
    }
}

void personal_info_new_phone_numbers(personal_info* info, char phones[][INFO_LEN], int count)
{
    if (!info) return;
    int limit = count < INFO_MAX_COUNT ? count : INFO_MAX_COUNT;
    info->phone_count = limit;
    for (int i = 0; i < limit; ++i) {
        strncpy(info->phone_numbers[i], phones[i], INFO_LEN - 1);
        info->phone_numbers[i][INFO_LEN - 1] = '\0';
    }
    for (int i = limit; i < INFO_MAX_COUNT; ++i) {
        info->phone_numbers[i][0] = '\0';
    }
}

void personal_info_new_links(personal_info* info, char links[][INFO_LEN], int count)
{
    if (!info) return;
    int limit = count < INFO_MAX_COUNT ? count : INFO_MAX_COUNT;
    info->link_count = limit;
    for (int i = 0; i < limit; ++i) {
        strncpy(info->links[i], links[i], INFO_LEN - 1);
        info->links[i][INFO_LEN - 1] = '\0';
    }
    for (int i = limit; i < INFO_MAX_COUNT; ++i) {
        info->links[i][0] = '\0';
    }
}

void personal_info_print_field(const char* field_name, const char* field)
{
    printf("%s: %s\n", field_name, field);
}

void personal_info_print_array(const char* array_name, const char arr[][INFO_LEN], int count)
{
    printf("%s:\n", array_name);
    if (count == 0) {
        printf("  (none)\n");
        return;
    }
    for (int i = 0; i < count; ++i) {
        printf("  %s\n", arr[i]);
    }
}

void personal_info_print_all(const personal_info* info)
{
    if (!info) return;
    personal_info_print_field("Name", info->name);
    personal_info_print_field("Surname", info->surname);
    personal_info_print_field("Patronymic", info->patronymic);
    personal_info_print_field("Workplace", info->workplace);
    personal_info_print_field("Position", info->position);
    personal_info_print_array("Phone Numbers", info->phone_numbers, info->phone_count);
    personal_info_print_array("Links", info->links, info->link_count);
}
