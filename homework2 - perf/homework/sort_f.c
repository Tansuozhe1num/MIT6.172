#include "./util.h"

#define INSERTION_SORT_THRESHOLD 16

void isort(data_t* begin, data_t* end);
static inline void copy_f(const data_t* source, data_t* dest, int n);
static inline void merge_f(data_t* A, int p, int q, int r, data_t* temp);
static void sort_f_rec(data_t* A, int p, int r, data_t* temp);

// 对外接口：只分配一次
void sort_f(data_t* A, int p, int r) {
  assert(A);
  if (p >= r) return;

  int n = r - p + 1;

  // 临时缓冲区大小至少要能容纳最大左半边
  data_t* temp = NULL;
  mem_alloc(&temp, n / 2 + 1);
  if (temp == NULL) {
    return;
  }

  sort_f_rec(A, p, r, temp);

  mem_free(&temp);
}

// 递归辅助函数
static void sort_f_rec(data_t* A, int p, int r, data_t* temp) {
  assert(A);
  assert(temp);

  int n = r - p + 1;
  if (n <= INSERTION_SORT_THRESHOLD) {
    isort(A + p, A + r);
    return;
  }

  int q = (p + r) / 2;
  sort_f_rec(A, p, q, temp);
  sort_f_rec(A, q + 1, r, temp);
  merge_f(A, p, q, r, temp);
}

// 只拷贝左半边到 temp，右半边留在 A 里
static inline void merge_f(data_t* A, int p, int q, int r, data_t* temp) {
  assert(A);
  assert(temp);
  assert(p <= q);
  assert(q + 1 <= r);

  int n1 = q - p + 1;
  copy_f(A + p, temp, n1);

  int i = n1 - 1;   // temp 指针末尾
  int j = r;        // A 中右半边末尾
  int k = r;        // 写回位置

  while (i >= 0 && j >= q + 1) {
    if (temp[i] > A[j]) {
      A[k--] = temp[i--];
    } else {
      A[k--] = A[j--];
    }
  }

  while (i >= 0) {
    A[k--] = temp[i--];
  }
}

static inline void copy_f(const data_t* source, data_t* dest, int n) {
  assert(dest);
  assert(source);

  for (int i = 0; i < n; ++i) {
    dest[i] = source[i];
  }
}