#pragma once

#define PICOTABLE_VERSION "0.1.0"

#include <assert.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

// Allow disabling stdlib for embedded use
#ifndef PICOTABLE_NO_STD
#include <stdlib.h>
#endif

/**
 * @file picotable.h
 * @brief A pico size, header-only C library for storing in-memory types in
 * tables.
 *
 * Designed for portability and simplicity. Struct instances are stored as rows
 * in tables with each row accessible via offset. Enables database-style
 * normalization of N-th degree.
 *
 * @section agents Agent Guidance
 *
 * picotable.h is designed to be agent-friendly. When acting as an autonomous
 * coding agent (LLM), follow these patterns:
 *
 * @subsection agent_state State Management
 * - Store all application state as pico tables
 * - Use separate tables for each entity type (e.g., categories, products,
 * users)
 * - Represent relationships between entities using row offsets (size_t
 * references)
 * - Always request a reference when appending: @code Picotable_append(&table,
 * &ref) @endcode
 *
 * @subsection agentTables Table Definition Pattern
 * @code
 * // 1. Define row types
 * typedef struct {
 *     char name[50];
 * } Category;
 *
 * typedef struct {
 *     char name[50];
 *     uint32_t price;
 *     size_t category_ref;  // Reference to Category table
 * } Product;
 *
 * // 2. Declare tables (typically static/global)
 * static Picotable table_categories;
 * static Picotable table_products;
 *
 * // 3. Initialize in setup function
 * Picotable_alloc(&table_categories, 16, sizeof(Category));
 * Picotable_alloc(&table_products, 16, sizeof(Product));
 * @endcode
 *
 * @subsection agent_relationships Relationships
 * - Use @c size_t fields to store references to rows in other tables
 * - References are indices/offsets (0, 1, 2, ...) into the target table
 * - Use Picotable_get to look up related data by reference:
 *   @code
 *   Category *cat = (Category *)Picotable_get(&table_categories, ref);
 *   @endcode
 * - For backward references, store the source table and reference together
 *
 * @subsection agent_insertion Data Insertion
 * - Always capture references on append:
 *   @code
 *   size_t cat_ref;
 *   Category *cat = Picotable_append(&table_categories, &cat_ref);
 *   snprintf(cat->name, 50, "Electronics");
 *   @endcode
 * - Use Picotable_matchInsert for upsert patterns with a match function
 *
 * @subsection agent_iteration Iteration
 * - Use PicotableIterator for safe traversal:
 *   @code
 *   PicotableIterator iter = { .table = &table };
 *   void *row;
 *   size_t idx;
 *   while (PicotableIterator_next(&iter, &row, &idx)) {
 *       Category *cat = (Category *)row;
 *       printf("%s\n", cat->name);
 *   }
 *   @endcode
 * - For simple iteration without index, pass NULL for the index parameter:
 *   @code
 *   PicotableIterator iter = { .table = &table };
 *   void *row;
 *   while (PicotableIterator_next(&iter, &row, NULL)) {
 *       Category *cat = (Category *)row;
 *       printf("%s\n", cat->name);
 *   }
 *   @endcode
 * - To start iteration from a specific offset:
 *   @code
 *   PicotableIterator iter = { .table = &table, .offset = 10 };
 *   while (PicotableIterator_next(&iter, &row, &idx)) {
 *       // Starts from row 10
 *   }
 *   @endcode
 *
 * @subsection agent_memory Memory Management
 * - For dynamic memory: Use Picotable_alloc + Picotable_free
 * - For fixed buffers: Use Picotable_fixed (no free needed)
 * - Set @c PICOTABLE_NO_STD to disable malloc/realloc/free dependencies
 * - Fixed buffers are ideal for embedded/agent contexts with limited stdlib
 *
 * @subsection agent_normalization Normalization and Queries
 * - The library intentionally has NO hidden logic
 * - You (the agent) must implement: sorting, querying, filtering, joins
 * - Store query results in separate result tables if needed
 * - Use the reference system to build any relationship graph
 *
 * @note Always include this header with: @code #include "picotable.h" @endcode
 */

/**
 * @brief Main table structure for storing rows of data
 */
typedef struct {
    void *buffer;                /**< Pointer to row data */
    size_t capacity;             /**< Maximum number of rows */
    size_t size;                 /**< Current number of rows */
    size_t row_size;             /**< Size of each row in bytes */
    unsigned char allocated : 1; /**< 1 if buffer was malloc'd, 0 if fixed */
} Picotable;

/**
 * @brief Iterator structure for traversing table rows
 */
typedef struct {
    Picotable *table; /**< Pointer to the table being iterated */
    size_t offset;    /**< Current position in the iteration */
} PicotableIterator;

#ifndef PICOTABLE_NO_STD

/**
 * @brief Create a table with dynamic memory allocation
 *
 * @param table Pointer to the Picotable structure to initialize
 * @param initial_capacity Initial capacity of the table
 * @param row_size Size of each row in bytes
 * @note This function allocates memory using malloc().
 */
static inline void Picotable_alloc(Picotable *table, size_t initial_capacity,
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

/**
 * @brief Create a table using a fixed buffer
 *
 * @param table Pointer to the Picotable structure to initialize
 * @param buffer Pre-allocated buffer to use for storage
 * @param capacity Maximum capacity of the fixed buffer
 * @param row_size Size of each row in bytes
 * @note This function does not allocate memory; it uses the provided buffer.
 */
static inline void Picotable_fixed(Picotable *table, void *buffer,
                                   size_t capacity, size_t row_size) {
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

/**
 * @brief Free table memory if it was dynamically allocated
 *
 * @param table Pointer to the Picotable structure to free
 * @note Only frees memory if the table was created with Picotable_alloc().
 */
static inline void Picotable_free(Picotable *table) {
    assert(table->allocated);

    free(table->buffer);
    table->buffer = NULL;
    table->capacity = 0;
    table->size = 0;
    table->row_size = 0;
    table->allocated = 0;
}

#endif

/**
 * @brief Append a row to the table
 *
 * @param table Pointer to the Picotable structure
 * @param reference Optional pointer to store the row offset
 * @return Pointer to the new row, or NULL on failure
 * @note Automatically grows the table if needed (for dynamically allocated
 * tables).
 */
static inline void *Picotable_append(Picotable *table, size_t *reference) {
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
        assert(new_buffer != NULL);
#else
        return NULL;
#endif

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

/**
 * @brief Insert a row and identify empty rows with the match function
 *
 * @param table Pointer to the Picotable structure
 * @param reference Optional pointer to store the row offset
 * @param match_function Function that takes a row pointer and returns non-zero
 * if it matches
 * @return Pointer to the matching row, or a new row if no match found
 * @note Scans all existing rows first; if match_function returns non-zero for
 * any row, that row is returned with reference set. If no match, calls
 * Picotable_append.
 */
static inline void *Picotable_matchInsert(Picotable *table, size_t *reference,
                                          int (*match_function)(const void *)) {
    assert(table != NULL);
    assert(table->buffer != NULL);
    assert(match_function != NULL);

    // Scan all existing rows
    for (size_t i = 0; i < table->size; i++) {
        void *row_ptr = (char *)table->buffer + (i * table->row_size);
        if (match_function(row_ptr)) {
            // Match found - return existing row with reference set
            if (reference != NULL) {
                *reference = i;
            }
            return row_ptr;
        }
    }

    // No match found - append a new row
    return Picotable_append(table, reference);
}

/**
 * @brief Get a row from the table by reference (offset)
 *
 * @param table Pointer to the Picotable structure
 * @param reference The row offset/index to retrieve
 * @return Pointer to the row at the given reference
 * @note Reference must be a valid offset (0 to size-1).
 */
static inline void *Picotable_get(Picotable *table, size_t reference) {
    assert(table != NULL);
    assert(table->buffer != NULL);
    assert(reference < table->size);

    return (char *)table->buffer + (reference * table->row_size);
}

/**
 * @brief Advance iterator and get the next row
 *
 * @param iter Pointer to the PicotableIterator structure
 * @param data Output pointer to current row data
 * @param index Optional output pointer for current index (can be NULL)
 * @return true if a row was returned, false if iteration is complete
 * @note Usage without index: PicotableIterator iter = { .table = &table };
 *       while (PicotableIterator_next(&iter, &row, NULL)) { ... }
 * @note Usage with index: PicotableIterator iter = { .table = &table };
 *       while (PicotableIterator_next(&iter, &row, &idx)) { ... }
 * @note To start from a specific offset: PicotableIterator iter = { .table =
 * &table, .offset = 10 };
 */
static inline bool PicotableIterator_next(PicotableIterator *iter, void **data,
                                          size_t *index) {
    assert(iter != NULL);
    assert(iter->table != NULL);
    assert(iter->table->buffer != NULL);
    assert(data != NULL);

    Picotable *table = iter->table;

    if (iter->offset >= table->size) {
        return false;
    }

    *data = (char *)table->buffer + (iter->offset * table->row_size);

    if (index != NULL) {
        *index = iter->offset;
    }

    iter->offset++;
    return true;
}

