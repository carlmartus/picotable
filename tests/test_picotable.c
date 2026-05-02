#include <criterion/criterion.h>

#include "../picotable.h"

TestSuite(picotable, .init = NULL, .fini = NULL);

Test(picotable, test_alloc_and_free) {
    Picotable table;
    size_t initial_capacity = 10;
    size_t row_size = sizeof(int);

    // Test allocation
    Picotable_alloc(&table, initial_capacity, row_size);
    cr_assert_not_null(table.buffer, "Buffer should be allocated");
    cr_assert_eq(table.capacity, initial_capacity,
                 "Capacity should match initial capacity");
    cr_assert_eq(table.size, 0, "Size should be 0 initially");
    cr_assert_eq(table.row_size, row_size, "Row size should match");
    cr_assert_eq(table.allocated, 1, "Allocated flag should be set");

    // Test free
    Picotable_free(&table);
    cr_assert_null(table.buffer, "Buffer should be NULL after free");
    cr_assert_eq(table.capacity, 0, "Capacity should be 0 after free");
    cr_assert_eq(table.size, 0, "Size should be 0 after free");
    cr_assert_eq(table.row_size, 0, "Row size should be 0 after free");
    cr_assert_eq(table.allocated, 0, "Allocated flag should be cleared");
}

Test(picotable, test_append_and_grow) {
    Picotable table;
    size_t initial_capacity = 4;
    size_t row_size = sizeof(int);

    // Create table with small initial capacity
    Picotable_alloc(&table, initial_capacity, row_size);

    // Fill table to capacity
    for (size_t i = 0; i < initial_capacity; i++) {
        int *row = (int *)Picotable_append(&table, NULL);
        cr_assert_not_null(row, "Append should succeed");
        *row = (int)i;
    }

    cr_assert_eq(table.size, initial_capacity,
                 "Size should equal initial capacity");
    cr_assert_eq(table.capacity, initial_capacity,
                 "Capacity should still be initial capacity");

    // This append should trigger growth
    int *new_row = (int *)Picotable_append(&table, NULL);
    cr_assert_not_null(new_row, "Append should succeed and trigger growth");
    cr_assert_eq(table.size, initial_capacity + 1,
                 "Size should be initial capacity + 1");
    cr_assert_eq(table.capacity, initial_capacity * 2,
                 "Capacity should double");

    // Verify the new row is writable
    *new_row = 42;
    cr_assert_eq(*new_row, 42, "New row should be writable");

    Picotable_free(&table);
}

Test(picotable, test_fixed_buffer) {
    Picotable table;
    size_t capacity = 5;
    size_t row_size = sizeof(int);
    int buffer[5];  // Fixed buffer

    // Create table with fixed buffer
    Picotable_fixed(&table, buffer, capacity, row_size);
    cr_assert_eq(table.buffer, buffer, "Buffer should match provided buffer");
    cr_assert_eq(table.capacity, capacity, "Capacity should match");
    cr_assert_eq(table.size, 0, "Size should be 0 initially");
    cr_assert_eq(table.row_size, row_size, "Row size should match");
    cr_assert_eq(table.allocated, 0, "Allocated flag should not be set");

    // Fill fixed buffer
    for (size_t i = 0; i < capacity; i++) {
        int *row = (int *)Picotable_append(&table, NULL);
        cr_assert_not_null(row, "Append should succeed within capacity");
        *row = (int)i;
    }

    // Try to append beyond capacity - should fail
    int *overflow_row = (int *)Picotable_append(&table, NULL);
    cr_assert_null(overflow_row,
                   "Append should fail when fixed buffer is full");
    cr_assert_eq(table.size, capacity, "Size should remain at capacity");
}

// Helper match function for testing match_insert
static int match_int_value(const void *row) {
    int value = *(const int *)row;
    return value == 42;
}

Test(picotable, test_match_insert) {
    Picotable table;
    size_t initial_capacity = 4;
    size_t row_size = sizeof(int);

    Picotable_alloc(&table, initial_capacity, row_size);

    // Add some rows with values 1, 2, 3, 4
    for (int i = 1; i <= 4; i++) {
        int *row = (int *)Picotable_append(&table, NULL);
        cr_assert_not_null(row, "Append should succeed");
        *row = i;
    }

    // Test match_insert with a value that doesn't exist - should append
    size_t new_reference;
    int *new_row =
        (int *)Picotable_match_insert(&table, &new_reference, match_int_value);
    cr_assert_not_null(new_row, "match_insert should append when no match");
    cr_assert_eq(new_reference, 4, "Reference should be index 4");
    cr_assert_eq(table.size, 5, "Size should be 5");

    // Set the new row value to 42
    *new_row = 42;

    // Test match_insert with a value that exists - should find match
    size_t found_reference;
    int *found_row = (int *)Picotable_match_insert(&table, &found_reference,
                                                   match_int_value);
    cr_assert_not_null(found_row, "match_insert should find match");
    cr_assert_eq(found_reference, 4,
                 "Reference should be index 4 (the 42 row)");
    cr_assert_eq(*found_row, 42, "Found row should have value 42");
    cr_assert_eq(table.size, 5, "Size should still be 5 (no new row added)");

    // Test with NULL reference
    int *found_row2 =
        (int *)Picotable_match_insert(&table, NULL, match_int_value);
    cr_assert_not_null(found_row2,
                       "match_insert should work with NULL reference");
    cr_assert_eq(*found_row2, 42, "Found row should still have value 42");

    Picotable_free(&table);
}

Test(picotable, test_iterate) {
    Picotable table;
    size_t initial_capacity = 5;
    size_t row_size = sizeof(int);

    Picotable_alloc(&table, initial_capacity, row_size);

    // Add rows with values 10, 20, 30
    for (int i = 1; i <= 3; i++) {
        int *row = (int *)Picotable_append(&table, NULL);
        *row = i * 10;
    }

    // Iterate and verify all rows
    size_t idx = 0;
    int expected = 10;
    void *row_ptr;
    while (Picotable_iterate(&table, &row_ptr, &idx)) {
        int *row = (int *)row_ptr;
        cr_assert_eq(*row, expected, "Row value should be %d", expected);
        expected += 10;
    }
    cr_assert_eq(idx, 3, "Index should be 3 after iterating all rows");

    // Test iteration on empty table
    Picotable_free(&table);
    Picotable_alloc(&table, 10, sizeof(int));

    idx = 0;
    int iteration_count = 0;
    while (Picotable_iterate(&table, &row_ptr, &idx)) {
        iteration_count++;
    }
    cr_assert_eq(iteration_count, 0, "Should not iterate on empty table");

    Picotable_free(&table);
}

Test(picotable, test_get) {
    Picotable table;
    size_t initial_capacity = 5;
    size_t row_size = sizeof(int);

    Picotable_alloc(&table, initial_capacity, row_size);

    // Add rows with values 100, 200, 300
    size_t refs[3];
    for (int i = 0; i < 3; i++) {
        int *row = (int *)Picotable_append(&table, &refs[i]);
        *row = (i + 1) * 100;
    }

    // Test getting rows by reference
    for (int i = 0; i < 3; i++) {
        int *row = (int *)Picotable_get(&table, refs[i]);
        cr_assert_not_null(row, "Picotable_get should return non-NULL");
        cr_assert_eq(*row, (i + 1) * 100,
                     "Row value at reference %zu should be %d", refs[i],
                     (i + 1) * 100);
    }

    // Test getting first and last row explicitly
    int *first_row = (int *)Picotable_get(&table, 0);
    cr_assert_eq(*first_row, 100, "First row should be 100");

    int *last_row = (int *)Picotable_get(&table, 2);
    cr_assert_eq(*last_row, 300, "Last row should be 300");

    Picotable_free(&table);
}

