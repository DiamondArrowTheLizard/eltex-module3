#include "array_contacts.h"
#include "ui.h"

int main()
{
    array_contacts* array = array_contacts_init();
    if (!array) return 1;

    array_contacts_load(array, "contacts.dat");

    ui_run(array);

    array_contacts_save(array, "contacts.dat");
    array_contacts_destroy(array);
    return 0;
}
