/*
 * Copyright (c) 2024 Jona Heitzer
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the “Software”), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED “AS IS”, WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#ifndef ARENA_H_
#define ARENA_H_

#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define ARENA_CAPACITY 0x100
#define ARENA_ALIGNMENT 8

typedef struct arena Arena;
typedef struct a_string AString;

// Arena stuff
Arena *A_create(const size_t sz);
void A_destroy(Arena *a);
void *A_alloc(Arena *a, const size_t sz);
void *A_calloc(Arena *a, const size_t cnt, const size_t sz);
void *A_realloc(Arena *a, void *ptr, const size_t sz);
void A_free(Arena *a, const void *ptr);
void A_debug_draw_graph(const Arena *a);

// String stuff
AString *AS_create_from_char_array(Arena *a, const char *carr, const size_t sz);
AString *AS_create_from_cstr(Arena *a, const char *str);
void AS_free(Arena *a, AString *astr);
AString *AS_append_char_array(Arena *a, AString *astr, const char *carr,
                              const size_t sz);
AString *AS_append_cstr(Arena *a, AString *astr, const char *str);
char *AS_data(AString *astr);
size_t AS_sz(AString *astr);

// String printf macros
#define AS_format(astr) "%.*s"
#define AS_arg(astr) (int)AS_sz(astr), AS_data(astr)

#ifdef ARENA_IMPLEMENTATION

#define _A_align(x)                                                            \
        ((x) % ARENA_ALIGNMENT ? (x + ARENA_ALIGNMENT - x % ARENA_ALIGNMENT)   \
                               : x)

struct chunk {
        size_t sz;
        struct chunk *next;
        char data[];
};

struct arena {
        size_t sz;
        struct chunk *free;
        struct chunk *used;
};

Arena *A_create(const size_t sz)
{
        Arena *a = malloc(sz);
        if (!a) {
                return NULL;
        }

        a->sz = sz;
        a->free = (struct chunk *)(a + 1);
        a->used = NULL;
        a->free->sz = sz - sizeof(*a) - sizeof(*a->free);
        a->free->next = NULL;

        return a;
}

void A_destroy(Arena *a) { free(a); }

// This cannot simply be forced to be inline with clang using the "inline"
// keyword, see https://clang.llvm.org/compatibility.html
bool _A_split_chunk(struct chunk *c, const size_t sz)
{
        if (!c || sz < 1 || c->sz < sizeof(*c) + _A_align(sz) + _A_align(1)) {
                return false;
        }

        struct chunk *tmp = (struct chunk *)(c->data + _A_align(sz));
        tmp->sz = c->sz - sizeof(*c) - _A_align(sz);
        tmp->next = c->next;
        c->sz = _A_align(sz);
        c->next = tmp;

        return true;
}

void *A_alloc(Arena *a, const size_t sz)
{
        if (!a || sz < 1) {
                return NULL;
        }

        // Find chunk in free list to match the allocation
        struct chunk **c = &a->free;
        while (*c && (*c)->sz < _A_align(sz)) {
                c = &(*c)->next;
        }

        // If no matching free chunk is found, bail out
        if (!*c) {
                return NULL;
        }

        // Split chunk if possible
        _A_split_chunk(*c, sz);

        // Remember (*c)->data, this is the allocated data to be returned to
        // the caller
        void *ret = (*c)->data;

        // Remove allocated chunk from free list and prepend to used list
        struct chunk *used = a->used;
        a->used = *c;
        *c = (*c)->next;
        a->used->next = used;

        return ret;
}

void *A_calloc(Arena *a, const size_t cnt, const size_t sz)
{
        void *m = A_alloc(a, cnt * sz);
        if (!m) {
                return NULL;
        }

        memset(m, 0, sz);

        return m;
}

void *A_realloc(Arena *a, void *ptr, const size_t sz)
{
        if (!a || !ptr || sz < 1) {
                return ptr;
        }

        // Find matching chunk to be realloc'd
        struct chunk *c = a->used;
        while (c && c->data != ptr) {
                c = c->next;
        }

        // If no chunk matches, do nothing and return NULL to signal failure to
        // caller
        if (!c) {
                return NULL;
        }

        // If new size fits current allocation do nothing and return ptr to
        // signal success to caller to caller
        if (c->sz >= sz) {
                return ptr;
        }

        void *new = A_alloc(a, sz);

        // If no new chunk can be allocated, do nothing and return NULL to
        // signal failure to caller
        if (!new) {
                return NULL;
        }

        // Copy data to new allocation
        memcpy(new, c->data, sz);

        // Free original allocation
        A_free(a, ptr);

        return new;
}

void A_free(Arena *a, const void *ptr)
{
        if (!a || !ptr) {
                return;
        }

        // Find matching chunk to be free'd in used list
        struct chunk **c = &a->used;
        while (*c && (*c)->data != ptr) {
                c = &(*c)->next;
        }

        // If no chunk matches, do nothing
        if (!*c) {
                return;
        }

        // Find last chunk in free list and append free'd chunk
        struct chunk *free = a->free;
        while (free && free->next) {
                free = free->next;
        }
        free->next = *c;

        // Remove chunk from used list
        *c = (*c)->next;

        // Set next of free'd chunk to NULL to terminate the free list properly
        free->next->next = NULL;
}

void A_debug_draw_graph(const Arena *a)
{
        if (!a) {
                return;
        }

        printf("Arena: %p, sz: %lu: free: %p, used: %p\n", (void *)a, a->sz,
               (void *)a->free, (void *)a->used);

        printf("Free: %p\n", (void *)a->free);
        for (struct chunk *c = a->free; c; c = c->next) {
                printf("    Chunk: %p, sz: %lu, next: %p, data: ", (void *)c,
                       c->sz, (void *)c->next);
                for (size_t i = 0; i < c->sz; i++) {
                        putchar(c->data[i] > 0x20 && c->data[1] < 0x7f
                                    ? c->data[i]
                                    : '.');
                }
                putchar('\n');
        }

        printf("Used: %p\n", (void *)a->used);
        for (struct chunk *c = a->used; c; c = c->next) {
                printf("    Chunk: %p, sz: %lu, next: %p, data: ", (void *)c,
                       c->sz, (void *)c->next);
                for (size_t i = 0; i < c->sz; i++) {
                        putchar(c->data[i] > 0x20 && c->data[1] < 0x7f
                                    ? c->data[i]
                                    : '.');
                }
                putchar('\n');
        }

        printf("\n------------------\n\n");
}

struct a_string {
        size_t sz;
        char data[];
};

AString *AS_create_from_char_array(Arena *a, const char *carr, const size_t sz)
{
        AString *astr = A_alloc(a, sizeof(AString) + sz);
        if (!astr) {
                return NULL;
        }

        astr->sz = sz;
        memcpy(astr->data, carr, astr->sz);

        return astr;
}

AString *AS_create_from_cstr(Arena *a, const char *str)
{
        return AS_create_from_char_array(a, str, strlen(str));
}

void AS_free(Arena *a, AString *astr) { A_free(a, astr); }

AString *AS_append_char_array(Arena *a, AString *astr, const char *carr,
                              const size_t sz)
{
        AString *realloced = A_realloc(a, astr, sizeof(*astr) + astr->sz + sz);
        if (!realloced) {
                return NULL;
        }

        // Since realloc already copied the original string to the new
        // allocation, only the string to be appended has to be copied
        memcpy(&realloced->data[realloced->sz], carr, sz);

        // Update size of astr
        size_t newsz = realloced->sz + sz;
        realloced->sz = newsz;

        return realloced;
}

AString *AS_append_cstr(Arena *a, AString *astr, const char *str)
{
        return AS_append_char_array(a, astr, str, strlen(str));
}

char *AS_data(AString *astr) { return astr->data; }

size_t AS_sz(AString *astr) { return astr->sz; }

#endif // ARENA_IMPLEMENTATION
#endif // ARENA_H_
