// A REPL that uses Picotable_matchInsert to upsert fruits into a Fruits table
// and PicotableIterator to display all entries with counts after each input.

#include <stdio.h>
#include <string.h>

#include "picotable.h"

typedef struct {
    char name[20];
    int count;
} Fruit;

static Picotable table_fruits;
static char current_input[20];

int fruit_match(const void *row) {
    Fruit *f = (Fruit *)row;
    return strcmp(f->name, current_input) == 0;
}

int main() {
    Picotable_alloc(&table_fruits, 8, sizeof(Fruit));

    while (1) {
        printf("Input fruit name: ");
        fgets(current_input, 20, stdin);
        current_input[strcspn(current_input, "\n")] = 0;

        if (current_input[0] == 0) {
            break;
        }

        size_t ref;
        Fruit *fruit =
            (Fruit *)Picotable_matchInsert(&table_fruits, &ref, fruit_match);

        if (fruit->count == 0) {
            strncpy(fruit->name, current_input, 19);
            fruit->name[19] = 0;
            fruit->count = 1;
        } else {
            fruit->count++;
        }

        printf("Fruits:\n");
        PicotableIterator iter = PicotableIterator_new(&table_fruits);
        Fruit *f;
        while (PicotableIterator_next(&iter, (void **)&f, NULL)) {
            printf("  %s: %d\n", f->name, f->count);
        }
    }

    Picotable_free(&table_fruits);
    return 0;
}

