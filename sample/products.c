#include <ncurses.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../picotable.h"

// Define the data structures
typedef struct {
    char name[20];
} Category;

typedef size_t CategoryOffset;

typedef struct {
    char name[20];
    CategoryOffset category_offset;
} Product;

// Global tables
Picotable categories_table;
Picotable products_table;

// Function prototypes
void init_tables();
void cleanup_tables();
void show_main_menu();
void add_category();
void add_product();
void list_products();
void draw_menu(const char *title, const char *options[], int count,
               int selected);
char *get_string_input(const char *prompt, char *buffer, size_t max_length);
CategoryOffset select_category();
void show_message(const char *message);

int main() {
    // Initialize ncurses
    initscr();
    cbreak();
    noecho();
    keypad(stdscr, TRUE);
    curs_set(0);

    // Initialize tables
    init_tables();

    // Main loop
    show_main_menu();

    // Cleanup
    cleanup_tables();
    endwin();

    return 0;
}

void init_tables() {
    // Initialize categories table
    Picotable_alloc(&categories_table, 10, sizeof(Category));

    // Initialize products table
    Picotable_alloc(&products_table, 20, sizeof(Product));
}

void cleanup_tables() {
    Picotable_free(&categories_table);
    Picotable_free(&products_table);
}

void draw_menu(const char *title, const char *options[], int count,
               int selected) {
    clear();
    mvprintw(2, (COLS - strlen(title)) / 2, "%s", title);

    for (int i = 0; i < count; i++) {
        if (i == selected) attron(A_REVERSE);
        mvprintw(5 + i * 2, (COLS - 20) / 2, "%d. %s", i + 1, options[i]);
        if (i == selected) attroff(A_REVERSE);
    }

    mvprintw(LINES - 3, (COLS - 30) / 2, "Use arrow keys and Enter to select");
    refresh();
}

char *get_string_input(const char *prompt, char *buffer, size_t max_length) {
    echo();
    curs_set(1);

    clear();
    mvprintw(LINES / 2, (COLS - strlen(prompt)) / 2, "%s: ", prompt);
    wgetnstr(stdscr, buffer, max_length - 1);

    noecho();
    curs_set(0);
    return buffer;
}

CategoryOffset select_category() {
    if (categories_table.size == 0) {
        return (CategoryOffset)-1;  // No categories available
    }

    size_t selected = 0;
    int ch;

    while (1) {
        clear();
        mvprintw(2, (COLS - 18) / 2, "Select Category");

        // Draw category list
        size_t i = 0;
        Category *cat;
        while (Picotable_iterate(&categories_table, (void **)&cat, NULL)) {
            if (i == selected) {
                attron(A_REVERSE);
            }
            mvprintw(5 + i * 2, (COLS - 30) / 2, "%zu. %s", i + 1, cat->name);
            if (i == selected) {
                attroff(A_REVERSE);
            }
            i++;
        }

        mvprintw(LINES - 3, (COLS - 35) / 2,
                 "Arrow keys to navigate, Enter to select, ESC to cancel");
        refresh();

        ch = getch();

        switch (ch) {
            case KEY_UP:
                if (selected > 0) selected--;
                break;
            case KEY_DOWN:
                if (selected < categories_table.size - 1) selected++;
                break;
            case 10:  // Enter
                return selected;
            case 27:  // ESC
                return (CategoryOffset)-1;
        }
    }
}

void show_message(const char *message) {
    clear();
    mvprintw(LINES / 2, (COLS - strlen(message)) / 2, "%s", message);
    refresh();
    napms(800);
}

void show_main_menu() {
    const char *options[] = {"Add Category", "Add Product", "List Inventory",
                             "Exit"};
    int selected = 0;

    while (1) {
        draw_menu("Product Management System", options, 4, selected);

        int ch = getch();
        switch (ch) {
            case KEY_UP:
                if (selected > 0) selected--;
                break;
            case KEY_DOWN:
                if (selected < 3) selected++;
                break;
            case 10:  // Enter
                switch (selected) {
                    case 0:
                        add_category();
                        break;
                    case 1:
                        add_product();
                        break;
                    case 2:
                        list_products();
                        break;
                    case 3:
                        return;
                }
                break;
        }
    }
}

void add_category() {
    Category new_category;
    get_string_input("Enter category name", new_category.name,
                     sizeof(new_category.name));

    if (strlen(new_category.name) > 0) {
        Category *cat = Picotable_append(&categories_table, NULL);
        strcpy(cat->name, new_category.name);
        show_message("Category added!");
    }
}

void add_product() {
    if (categories_table.size == 0) {
        show_message("Add a category first!");
        return;
    }

    CategoryOffset cat_offset = select_category();
    if (cat_offset == (CategoryOffset)-1) {
        show_message("Cancelled");
        return;
    }

    Product new_product;
    new_product.category_offset = cat_offset;
    get_string_input("Enter product name", new_product.name,
                     sizeof(new_product.name));

    if (strlen(new_product.name) > 0) {
        Product *prod = Picotable_append(&products_table, NULL);
        strcpy(prod->name, new_product.name);
        prod->category_offset = cat_offset;
        show_message("Product added!");
    }
}

void list_products() {
    clear();

    if (products_table.size == 0) {
        show_message("No products yet.");
        return;
    }

    mvprintw(2, (COLS - 20) / 2, "Product Inventory");
    mvprintw(4, 5, "%-20s %-20s", "Product", "Category");

    size_t i = 0;
    Product *prod;
    while (Picotable_iterate(&products_table, (void **)&prod, NULL)) {
        Category *cat =
            (Category *)Picotable_get(&categories_table, prod->category_offset);
        mvprintw(6 + i, 5, "%-20s %-20s", prod->name, cat->name);
        i++;
    }

    mvprintw(LINES - 3, (COLS - 30) / 2, "Press any key to continue");
    refresh();
    getch();
}

