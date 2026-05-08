![Logo](logo.png)

# picotable.h

A pico size, header-only C library for storing in-memory types in tables. Designed for portability.

Struct instances are stored as rows in tables with each row accessible with an offset. Suitable for the equivalent of [database normalization](https://en.wikipedia.org/wiki/Database_normalization) in an embedded setting. Although the library is pico size, it may serve as a central piece for enabling dynamic business logic in C programs.

## Features

- **Allocation**: Supports both dynamic (`malloc`) or fixed size buffers.
- **Agent friendly**: Header with built-in LLM instructions. Tell your agent to use `@picotable.h` and to store the application state as pico tables.
- **No Hidden Logic**: Normalization, sorting, and querying are left to the user. What did you expect? It's pico size.
- **No stdlib required**: Set the define `PICOTABLE_NO_STD` and stick to fixed buffers. Removing *clib* as a dependency.

## Installation

1. Copy `picotable.h` into your project.
2. Include it in your source files:

```c
#include "picotable.h"
```

## Quick Start

### 1. Define your row structures

```c
typedef struct {
    char name[20];
} Category;

typedef struct {
    char name[20];
    uint32_t price;
    uint32_t amount_in_stock;
    size_t category_reference; // Reference to Category
} Product;
```

### 2. Create tables

Dynamic, allocate initial capacity of 17 rows for each table:

```c
static Picotable table_categories;
static Picotable table_products;

Picotable_alloc(&table_categories, 17, sizeof(Category));
Picotable_alloc(&table_products, 17, sizeof(Product));
```

Or fixed size:

```c
static Picotable table_categories;
static Picotable table_products;

Category buf_category[17];
Product buf_product[17];

Picotable_fixed(&table_categories, buf_category, 17, sizeof(Category));
Picotable_fixed(&table_products, buf_product, 17, sizeof(Product));
```

### 3. Append Data

```c
size_t category_reference;
Category *category = Picotable_append(&table_categories, &category_reference);
snprintf(category->name, 20, "My category 1");
```

```c
Product *product = Picotable_append(&table_products, NULL);
snprintf(product->name, 20, "My product 1");
product->price = 13;
product->amount_in_stock = 7;
product->category_reference = category_reference;
```

### 4. Free Memory (if allocated)

```c
Picotable_free(&table_categories);
Picotable_free(&table_products);
```

## API Reference

> [!TIP]
> Each function has detailed Doxygen-style documentation inside `picotable.h` with parameters, return values, and usage notes.

Change library properties by setting these defines.

| Define | Description |
|:--|:--|
| `PICOTABLE_VERSION` | Picotable version stamp |
| `PICOTABLE_NO_STD` | Disable include of `stdlib.h` - allocations disabled |


| Function | Description |
|:--|:--|
| `Picotable_alloc` | Create a table with dynamic memory. |
| `Picotable_fixed` | Create a table that uses a fixed buffer. |
| `Picotable_free` | Free table with dynamic memory. |
| `Picotable_append` | Append a row; returns `NULL` on failure. |
| `Picotable_matchInsert` | Insert a row at the next empty space. |
| `Picotable_get` | Get a row by reference (offset). |
| `Picotable_truncate` | Truncate the table to a new size (reduce only). |
| `Picotable_count` | Get the current number of rows in the table. |
| `PicotableIterator_new` | Create a new iterator for a table. |
| `PicotableIterator_skip` | Skip ahead in iteration. |
| `PicotableIterator_next` | Advance iterator and get the next row. |

**Do:**
- Use separate tables for each entity type (categories, products, users).
- Represent relationships using row offsets (`size_t` references).
- Capture references when appending: `Picotable_append(..., &ref)` & `Picotable_matchInsert(..., &ref, ...)`.
- Use `Picotable_get` to look up related data by reference.
- Use `PicotableIterator` for safe traversal of table rows.
- Use fixed buffers (`Picotable_fixed`) for embedded contexts without stdlib.

**Don't:**
- Don't expect hidden logic — you must implement sorting, querying, filtering, and joins yourself.
- Don't forget to check for `NULL` returns from `Picotable_append` & `Picotable_matchInsert` on fixed buffers.
- Don't mix allocation modes for tables — use either dynamic (`Picotable_alloc` & `Picotable_free`) or fixed (`Picotable_fixed`), not both.
- Don't forget to free dynamically allocated tables with `Picotable_free`.

## Samples

Sample programs demonstrating picotable usage:

- **[fruit_counter.c](sample/fruit_counter.c)** - A REPL that uses `Picotable_matchInsert` to upsert fruits into a Fruits table and `PicotableIterator` to display all entries with counts after each input.
- **[products.c](sample/products.c)** - An ncurses-based menu application that uses two tables (categories and products) with a reference from products to categories.

To build all samples: `make samples`

