#include "ui.h"
#include "array_contacts.h"
#include "personal_info.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int get_line(char* buff, size_t sz)
{
    int ch, extra;
    if (fgets(buff, sz, stdin) == NULL)
        return UI_NO_INPUT;
    if (buff[strlen(buff) - 1] != '\n') {
        extra = 0;
        while (((ch = getchar()) != '\n') && (ch != EOF))
            extra = 1;
        return (extra == 1) ? UI_TOO_LONG : UI_OK;
    }
    buff[strlen(buff) - 1] = '\0';
    return UI_OK;
}

static void read_phone_numbers(char phones[][INFO_LEN], int* count)
{
    char buffer[INFO_LEN];
    *count = 0;
    while (*count < INFO_MAX_COUNT) {
        printf("> ");
        int res = get_line(buffer, sizeof(buffer));
        if (res != UI_OK) continue;
        if (strlen(buffer) == 0) break;
        strncpy(phones[*count], buffer, INFO_LEN - 1);
        phones[*count][INFO_LEN - 1] = '\0';
        (*count)++;
    }
}

static void read_links(char links[][INFO_LEN], int* count)
{
    char buffer[INFO_LEN];
    *count = 0;
    while (*count < INFO_MAX_COUNT) {
        printf("> ");
        int res = get_line(buffer, sizeof(buffer));
        if (res != UI_OK) continue;
        if (strlen(buffer) == 0) break;
        strncpy(links[*count], buffer, INFO_LEN - 1);
        links[*count][INFO_LEN - 1] = '\0';
        (*count)++;
    }
}

void ui_run(array_contacts* array)
{
    printf("Contact book\n");
    for (;;) {
        printf("\n1: Add new entry\n");
        printf("2: Redact entry\n");
        printf("3: Delete entry\n");
        printf("4: Print entry\n");
        printf("5: Print all entries\n");
        printf("6: Quit program\n");
        printf("==> ");

        char user_input[UI_USER_INPUT_SIZE];
        int res = get_line(user_input, sizeof(user_input));
        if (res == UI_NO_INPUT) continue;
        if (res == UI_TOO_LONG) {
            printf("Input is one number. Please try again.\n");
            continue;
        }
        int user_choice = atoi(user_input);
        int signal = ui_process_user_action(user_choice);
        switch (signal) {
            case UI_SIGNAL_ADD:      ui_add(array); break;
            case UI_SIGNAL_REDACT:   ui_redact(array); break;
            case UI_SIGNAL_DELETE:   ui_delete(array); break;
            case UI_SIGNAL_PRINT_ENTRY: ui_print_entry(array); break;
            case UI_SIGNAL_PRINT_ALL:   ui_print_all(array); break;
            case UI_SIGNAL_QUIT:     printf("Quitting program\n"); return;
            default:                 printf("Error: unknown signal\n");
        }
    }
}

int ui_process_user_action(int user_choice)
{
    switch (user_choice) {
        case 1: return UI_SIGNAL_ADD;
        case 2: return UI_SIGNAL_REDACT;
        case 3: return UI_SIGNAL_DELETE;
        case 4: return UI_SIGNAL_PRINT_ENTRY;
        case 5: return UI_SIGNAL_PRINT_ALL;
        case 6: return UI_SIGNAL_QUIT;
        default: return UI_SIGNAL_INPUT_ERROR;
    }
}

void ui_add(array_contacts* array)
{
    char name[INFO_LEN];
    char surname[INFO_LEN];
    printf("Enter name of the contact: ");
    if (get_line(name, sizeof(name)) != UI_OK) return;
    printf("Enter surname of the contact: ");
    if (get_line(surname, sizeof(surname)) != UI_OK) return;

    personal_info* new_info = personal_info_create(name, surname);
    if (!new_info) return;

    array_contacts_append(array, new_info);
    printf("New contact added:\n");
    array_contacts_print_at_index(array, array->size - 1);
}

void ui_redact(array_contacts* array)
{
    if (array->size == 0) {
        printf("No contacts to redact.\n");
        return;
    }
    printf("Array size: %zu\n", array->size);
    printf("Enter contact index (first is 0): ");
    char idx_str[20];
    if (get_line(idx_str, sizeof(idx_str)) != UI_OK) return;
    size_t index = (size_t)atoi(idx_str);
    if (index >= array->size) {
        printf("Index out of range.\n");
        return;
    }

    personal_info* info = array->contacts[index];
    printf("Editing contact:\n");
    personal_info_print_all(info);
    printf("\n");

    int done = 0;
    while (!done) {
        printf("Select field to edit:\n");
        printf("1: Name\n2: Surname\n3: Patronymic\n4: Workplace\n5: Position\n");
        printf("6: Phone Numbers\n7: Links\n8: Done\n==> ");
        char choice_str[UI_USER_INPUT_SIZE];
        if (get_line(choice_str, sizeof(choice_str)) != UI_OK) continue;
        int choice = atoi(choice_str);
        char buffer[INFO_LEN];
        switch (choice) {
            case 1:
                printf("New name: ");
                if (get_line(buffer, sizeof(buffer)) == UI_OK)
                    personal_info_new_name(info, buffer);
                break;
            case 2:
                printf("New surname: ");
                if (get_line(buffer, sizeof(buffer)) == UI_OK)
                    personal_info_new_surname(info, buffer);
                break;
            case 3:
                printf("New patronymic (empty to clear): ");
                if (get_line(buffer, sizeof(buffer)) == UI_OK) {
                    if (strlen(buffer) == 0) buffer[0] = '\0';
                    personal_info_new_patronymic(info, buffer);
                }
                break;
            case 4:
                printf("New workplace (empty to clear): ");
                if (get_line(buffer, sizeof(buffer)) == UI_OK) {
                    if (strlen(buffer) == 0) buffer[0] = '\0';
                    personal_info_new_workplace(info, buffer);
                }
                break;
            case 5:
                printf("New position (empty to clear): ");
                if (get_line(buffer, sizeof(buffer)) == UI_OK) {
                    if (strlen(buffer) == 0) buffer[0] = '\0';
                    personal_info_new_position(info, buffer);
                }
                break;
            case 6:
                printf("Enter new phone numbers (empty line to finish):\n");
                {
                    char phones[INFO_MAX_COUNT][INFO_LEN];
                    int cnt = 0;
                    read_phone_numbers(phones, &cnt);
                    personal_info_new_phone_numbers(info, phones, cnt);
                }
                break;
            case 7:
                printf("Enter new links (empty line to finish):\n");
                {
                    char links[INFO_MAX_COUNT][INFO_LEN];
                    int cnt = 0;
                    read_links(links, &cnt);
                    personal_info_new_links(info, links, cnt);
                }
                break;
            case 8:
                done = 1;
                printf("Redaction complete.\n");
                break;
            default:
                printf("Invalid choice.\n");
        }
        printf("\n");
    }
}

void ui_delete(array_contacts* array)
{
    if (array->size == 0) {
        printf("No contacts to delete.\n");
        return;
    }
    printf("Array size: %zu\n", array->size);
    printf("Enter contact index (first is 0): ");
    char idx_str[20];
    if (get_line(idx_str, sizeof(idx_str)) != UI_OK) return;
    size_t index = (size_t)atoi(idx_str);
    if (array_contacts_remove(array, index) == ARRAY_CONTACTS_OK)
        printf("Entry deleted.\n");
    else
        printf("Error: index out of range.\n");
}

void ui_print_entry(array_contacts* array)
{
    if (array->size == 0) {
        printf("No contacts to print.\n");
        return;
    }
    printf("Array size: %zu\n", array->size);
    printf("Enter contact index (first is 0): ");
    char idx_str[20];
    if (get_line(idx_str, sizeof(idx_str)) != UI_OK) return;
    size_t index = (size_t)atoi(idx_str);
    if (array_contacts_print_at_index(array, index) == ARRAY_CONTACTS_OK)
        printf("Contact printed.\n");
    else
        printf("Error: index out of range.\n");
}

void ui_print_all(array_contacts* array)
{
    array_contacts_print_all(array);
    printf("All contacts printed.\n");
}
