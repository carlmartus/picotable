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

