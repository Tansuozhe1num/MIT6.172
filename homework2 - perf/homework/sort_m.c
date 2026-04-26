/**
 * Copyright (c) 2012 MIT License by 6.172 Staff
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to
 * deal in the Software without restriction, including without limitation the
 * rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
 * sell copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 **/

#include "./util.h"

#define INSERTION_SORT_THRESHOLD 16

// Function prototypes
void isort(data_t* begin, data_t* end);
static inline void merge_m(data_t* A, int p, int q, int r);
static inline void copy_m(const data_t* source, data_t* dest, int n);

// Merge sort with coarsened recursion
void sort_m(data_t* A, int p, int r) {
  assert(A);

  if (p >= r) {
    return;
  }

  int n = r - p + 1;
  if (n <= INSERTION_SORT_THRESHOLD) {
    isort(A + p, A + r);
    return;
  }

  int q = (p + r) / 2;
  sort_m(A, p, q);
  sort_m(A, q + 1, r);
  merge_m(A, p, q, r);
}

// Merge using only one temporary buffer:
// copy left half to temp, keep right half in A
static inline void merge_m(data_t* A, int p, int q, int r) {
  assert(A);
  assert(p <= q);
  assert((q + 1) <= r);

  int n1 = q - p + 1;

  data_t* left = NULL;
  mem_alloc(&left, n1);

  if (left == NULL) {
    mem_free(&left);
    return;
  }

  // Copy only the left half into temp buffer
  copy_m(A + p, left, n1);

  // Merge from the back to avoid overwriting unread right-half elements
  int i = n1 - 1;   // left temp index
  int j = r;        // right half index in A
  int k = r;        // write index in A

  while (i >= 0 && j >= q + 1) {
    if (left[i] > A[j]) {
      A[k--] = left[i--];
    } else {
      A[k--] = A[j--];
    }
  }

  // If left half still has leftovers, copy them back
  while (i >= 0) {
    A[p + i] = left[i];
    --i;
  }

  mem_free(&left);
}

static inline void copy_m(const data_t* source, data_t* dest, int n) {
  assert(dest);
  assert(source);

  const data_t* s = source;
  data_t* d = dest;

  for (int i = 0; i < n; ++i) {
    *d++ = *s++;
  }
}