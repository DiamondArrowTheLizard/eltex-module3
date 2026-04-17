#ifndef PERSONAL_INFO_H
#define PERSONAL_INFO_H

#define INFO_LEN 100
#define INFO_MAX_COUNT 20

typedef struct personal_info
{
    char name[INFO_LEN];
    char surname[INFO_LEN];
    char patronymic[INFO_LEN];
    char workplace[INFO_LEN];
    char position[INFO_LEN];

    char phone_numbers[INFO_MAX_COUNT][INFO_LEN];
    int phone_count;

    char links[INFO_MAX_COUNT][INFO_LEN];
    int link_count;

} personal_info;

personal_info* personal_info_create(const char* name, const char* surname);
void personal_info_destroy(personal_info* info);

void personal_info_new_name(personal_info* info, const char* name);
void personal_info_new_surname(personal_info* info, const char* surname);
void personal_info_new_patronymic(personal_info* info, const char* patronymic);
void personal_info_new_workplace(personal_info* info, const char* workplace);
void personal_info_new_position(personal_info* info, const char* position);
void personal_info_new_phone_numbers(personal_info* info, char phones[][INFO_LEN], int count);
void personal_info_new_links(personal_info* info, char links[][INFO_LEN], int count);

void personal_info_print_field(const char* field_name, const char* field);
void personal_info_print_array(const char* array_name, const char arr[][INFO_LEN], int count);
void personal_info_print_all(const personal_info* info);

#endif //!PERSONAL_INFO_H
