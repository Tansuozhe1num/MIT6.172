#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <cilk/cilk.h>

#define N 8
#define ALL ((1u << N) - 1u)
#define CUTOFF 4   // 可按需要调整，越大越偏串行，越小 spawn 越多

typedef struct Node {
    uint64_t sol;
    struct Node* next;
} Node;

typedef struct {
    Node* head;
    Node* tail;
    int size;
} BoardList;

static void init_list(BoardList* list) {
    list->head = NULL;
    list->tail = NULL;
    list->size = 0;
}

static void append_solution(BoardList* list, uint64_t sol) {
    Node* node = (Node*)malloc(sizeof(Node));
    if (node == NULL) {
        perror("malloc");
        exit(1);
    }
    node->sol = sol;
    node->next = NULL;

    if (list->tail == NULL) {
        list->head = list->tail = node;
    } else {
        list->tail->next = node;
        list->tail = node;
    }
    list->size++;
}

static void merge_lists(BoardList* list1, BoardList* list2) {
    if (list2->head == NULL) {
        init_list(list2);
        return;
    }

    if (list1->head == NULL) {
        *list1 = *list2;
        init_list(list2);
        return;
    }

    list1->tail->next = list2->head;
    list1->tail = list2->tail;
    list1->size += list2->size;
    init_list(list2);
}

static void free_list(BoardList* list) {
    Node* cur = list->head;
    while (cur != NULL) {
        Node* nxt = cur->next;
        free(cur);
        cur = nxt;
    }
    init_list(list);
}

static void print_solution(uint64_t sol) {
    for (int r = 0; r < N; ++r) {
        unsigned int place = (unsigned int)((sol >> (r * 8)) & 0xFFu);
        for (int c = 0; c < N; ++c) {
            putchar(place == (1u << c) ? 'Q' : '.');
        }
        putchar('\n');
    }
    putchar('\n');
}

static void try_serial(unsigned int row, unsigned int left, unsigned int right,
                       int depth, uint64_t sol, BoardList* out) {
    if (row == ALL) {
        append_solution(out, sol);
        return;
    }

    unsigned int poss = ~(row | left | right) & ALL;
    while (poss != 0) {
        unsigned int place = poss & -poss;
        poss &= ~place;

        try_serial(row | place,
                   (left | place) << 1,
                   (right | place) >> 1,
                   depth + 1,
                   sol | ((uint64_t)place << (depth * 8)),
                   out);
    }
}

static void try(unsigned int row, unsigned int left, unsigned int right,
                int depth, uint64_t sol, BoardList* out) {
    if (row == ALL) {
        append_solution(out, sol);
        return;
    }

    if (depth >= CUTOFF) {
        try_serial(row, left, right, depth, sol, out);
        return;
    }

    unsigned int poss = ~(row | left | right) & ALL;
    BoardList child_lists[N];
    int child_cnt = 0;

    while (poss != 0) {
        unsigned int place = poss & -poss;
        poss &= ~place;

        int idx = child_cnt++;
        init_list(&child_lists[idx]);

        cilk_spawn try(row | place,
                       (left | place) << 1,
                       (right | place) >> 1,
                       depth + 1,
                       sol | ((uint64_t)place << (depth * 8)),
                       &child_lists[idx]);
    }

    cilk_sync;

    for (int i = 0; i < child_cnt; ++i) {
        merge_lists(out, &child_lists[i]);
    }
}

int main(void) {
    BoardList ans;
    init_list(&ans);

    try(0, 0, 0, 0, 0, &ans);

    printf("There are %d solutions.\n", ans.size);

    // for (Node* p = ans.head; p != NULL; p = p->next) {
    //     print_solution(p->sol);
    // }

    free_list(&ans);
    return 0;
}