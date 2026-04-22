#ifndef PICOTABLE_H
#define PICOTABLE_H

#include <assert.h>
#include <stddef.h>
#include <stdint.h>

// Allow disabling stdlib for embedded use
#ifndef PICOTABLE_NO_STD
#include <stdlib.h>
#endif

// Main table structure
typedef struct {
    void *buffer;                 // Pointer to row data
    size_t capacity;              // Maximum number of rows
    size_t size;                  // Current number of rows
    size_t row_size;              // Size of each row in bytes
    unsigned char allocated : 1;  // 1 if buffer was malloc'd, 0 if fixed
} Picotable;

#ifndef PICOTABLE_NO_STD

void Picotable_alloc(Picotable *table, size_t initial_capacity,
                     size_t row_size) {
    assert(initial_capacity > 0);
    assert(row_size > 0);

    void *buffer = NULL;
    buffer = malloc(initial_capacity * row_size);

    table->buffer = buffer;
    table->capacity = initial_capacity;
    table->size = 0;
    table->row_size = row_size;
    table->allocated = 1;
}

#endif

// Create a table using a fixed buffer
void Picotable_fixed(Picotable *table, void *buffer, size_t capacity,
                     size_t row_size) {
    assert(table != NULL);
    assert(buffer != NULL);
    assert(capacity > 0);
    assert(row_size > 0);

    table->buffer = buffer;
    table->capacity = capacity;
    table->size = 0;
    table->row_size = row_size;
    table->allocated = 0;
}

#ifndef PICOTABLE_NO_STD

// Free table memory if it was dynamically allocated
void Picotable_free(Picotable *table) {
    assert(table->allocated);

    free(table->buffer);
    table->buffer = NULL;
    table->capacity = 0;
    table->size = 0;
    table->row_size = 0;
    table->allocated = 0;
}

#endif

// Append a row to the table
// If reference is not NULL, stores the row offset
// Returns pointer to the new row, or NULL on failure
void *Picotable_append(Picotable *table, size_t *reference) {
    assert(table->buffer != NULL);

    // Check if we need to grow the table
    if (table->size >= table->capacity) {
        // Only grow if we allocated the buffer
        if (!table->allocated) {
            return NULL;  // Fixed buffer is full
        }

        // Double the capacity
        size_t new_capacity = table->capacity * 2;
        void *new_buffer = NULL;

#ifndef PICOTABLE_NO_STD
        new_buffer = realloc(table->buffer, new_capacity * table->row_size);
#else
        return NULL;
#endif

        if (new_buffer == NULL) {
            return NULL;
        }

        table->buffer = new_buffer;
        table->capacity = new_capacity;
    }

    // Calculate pointer to new row
    void *row_ptr = (char *)table->buffer + (table->size * table->row_size);

    // Store reference if requested
    if (reference != NULL) {
        *reference = table->size;
    }

    table->size++;
    return row_ptr;
}

#endif  // PICOTABLE_H

