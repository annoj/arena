#define ARENA_IMPLEMENTATION
#include "arena.h"

#define TEST_IMPLEMENTATION
#include "test.h"

void can_create_arena(void)
{
        Arena *a = A_create(ARENA_CAPACITY);
        T_assert_is_not_null(a);
        A_destroy(a);
}

void can_alloc_char_buf(void)
{
        Arena *a = A_create(ARENA_CAPACITY);

        char *abuf = A_alloc(a, 5);
        memset(abuf, 'A', 5);
        char expected[] = "AAAAA";
        T_assert_char_array_equal((char *)&expected, abuf, 5);

        A_destroy(a);
}

void can_free_char_buf(void)
{
        // This cannot easily be tested, since A_free (much like clib free) does
        // not signal success/error to the caller
}

void can_alloc_two_char_bufs(void)
{
        Arena *a = A_create(ARENA_CAPACITY);

        char *abuf = A_alloc(a, 5);
        memset(abuf, 'A', 5);
        char expected_abuf[] = "AAAAA";
        T_assert_char_array_equal((char *)&expected_abuf, abuf, 5);

        char *bbuf = A_alloc(a, 3);
        memset(bbuf, 'B', 3);
        char expected_bbuf[] = "BBB";
        T_assert_char_array_equal((char *)&expected_bbuf, bbuf, 3);

        A_destroy(a);
}

void can_allocate_matching_freed_chunk(void)
{
        Arena *a = A_create(ARENA_CAPACITY);

        char *abuf = A_alloc(a, 8);
        memset(abuf, 'A', 8);
        char expected_abuf[] = "AAAAAAAA";
        T_assert_char_array_equal((char *)&expected_abuf, abuf, 8);

        A_free(a, abuf);

        char *bbuf = A_alloc(a, 8);
        memset(bbuf, 'B', 8);
        char expected_bbuf[] = "BBBBBBBB";
        T_assert_char_array_equal(expected_bbuf, bbuf, 8);

        A_destroy(a);
}

void can_allocate_nonmatching_freed_chunk(void)
{
        Arena *a = A_create(ARENA_CAPACITY);

        char *abuf = A_alloc(a, 8);
        memset(abuf, 'A', 8);
        char expected_abuf[] = "AAAAAAAA";
        T_assert_char_array_equal((char *)&expected_abuf, abuf, 8);

        A_free(a, abuf);

        char *bbuf = A_alloc(a, 9);
        memset(bbuf, 'B', 9);
        char expected_bbuf[] = "BBBBBBBBB";
        T_assert_char_array_equal(expected_bbuf, bbuf, 9);

        A_destroy(a);
}

void can_allocate_two_char_arrays_free_last_and_allocate_matching(void)
{
        Arena *a = A_create(ARENA_CAPACITY);

        char *abuf = A_alloc(a, 8);
        memset(abuf, 'A', 8);
        char expected_abuf[] = "AAAAAAAA";
        T_assert_char_array_equal((char *)&expected_abuf, abuf, 8);

        char *bbuf = A_alloc(a, 8);
        memset(bbuf, 'B', 8);
        char expected_bbuf[] = "BBBBBBBB";
        T_assert_char_array_equal(expected_bbuf, bbuf, 8);

        A_free(a, abuf);

        char *cbuf = A_alloc(a, 8);
        memset(bbuf, 'C', 8);
        char expected_cbuf[] = "CCCCCCCC";
        T_assert_char_array_equal(expected_cbuf, cbuf, 8);

        A_destroy(a);
}

void can_allocate_two_char_arrays_free_last_and_allocate_nonmatching(void)
{
        Arena *a = A_create(ARENA_CAPACITY);

        char *abuf = A_alloc(a, 8);
        memset(abuf, 'A', 8);
        char expected_abuf[] = "AAAAAAAA";
        T_assert_char_array_equal((char *)&expected_abuf, abuf, 8);

        char *bbuf = A_alloc(a, 8);
        memset(bbuf, 'B', 8);
        char expected_bbuf[] = "BBBBBBBB";
        T_assert_char_array_equal(expected_bbuf, bbuf, 8);

        A_free(a, abuf);

        char *cbuf = A_alloc(a, 9);
        memset(bbuf, 'C', 9);
        char expected_cbuf[] = "CCCCCCCCC";
        T_assert_char_array_equal(expected_cbuf, cbuf, 9);

        A_destroy(a);
}

void can_create_astring(void)
{
        Arena *a = A_create(ARENA_CAPACITY);

        AString *astr = AS_create_from_cstr(a, "ASDF");
        char expected[] = "ASDF";
        T_assert_char_array_equal(expected, astr->data, 4);

        A_destroy(a);
}

void can_append_to_astring(void)
{
        Arena *a = A_create(ARENA_CAPACITY);

        AString *astr = AS_create_from_cstr(a, "ASDF");
        char expected[] = "ASDF";
        T_assert_char_array_equal(expected, astr->data, 4);

        AS_append_cstr(a, astr, "asdf");
        char expected_appended[] = "ASDFasdf";
        T_assert_char_array_equal(astr->data, expected_appended, 8);

        A_destroy(a);
}

void can_append_to_astring_twice(void)
{
        Arena *a = A_create(ARENA_CAPACITY);

        AString *astr = AS_create_from_cstr(a, "ASDF");
        char expected[] = "ASDF";
        T_assert_char_array_equal(expected, astr->data, 4);

        AS_append_cstr(a, astr, "asdf");
        char expected_appended[] = "ASDFasdf";
        T_assert_char_array_equal(astr->data, expected_appended, 8);

        AS_append_cstr(a, astr, "AaSsDdFf");
        char expected_appended_appended[] = "ASDFasdfAaSsDdFf";
        T_assert_char_array_equal(astr->data, expected_appended_appended, 16);

        A_destroy(a);
}

int main(void)
{
        T_begin();

        T_run_test(can_create_arena);
        T_run_test(can_alloc_char_buf);
        T_run_test(can_free_char_buf);
        T_run_test(can_alloc_two_char_bufs);
        T_run_test(can_allocate_matching_freed_chunk);
        T_run_test(can_allocate_nonmatching_freed_chunk);
        T_run_test(
            can_allocate_two_char_arrays_free_last_and_allocate_matching);
        T_run_test(
            can_allocate_two_char_arrays_free_last_and_allocate_matching);
        T_run_test(can_create_astring);
        T_run_test(can_append_to_astring);
        T_run_test(can_append_to_astring_twice);

        return T_end();
}
