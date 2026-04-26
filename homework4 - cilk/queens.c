#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <cilk/cilk.h>
#include <cilk/reducer.h>

#define N 8
#define ALL ((1u << N) - 1u)
#define CUTOFF 4   // 可按需要调整

typedef struct Node {
    uint64_t sol;
    struct Node* next;
} Node;

typedef struct {
    Node* head;
    Node* tail;
    int size;
} BoardList;

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

static void board_list_identity(void* key, void* value) {
    (void)key;
    BoardList* list = (BoardList*)value;
    list->head = NULL;
    list->tail = NULL;
    list->size = 0;
}

static void board_list_reduce(void* key, void* left, void* right) {
    (void)key;
    BoardList* L = (BoardList*)left;
    BoardList* R = (BoardList*)right;

    if (R->head == NULL) {
        return;
    }

    if (L->head == NULL) {
        *L = *R;
        R->head = NULL;
        R->tail = NULL;
        R->size = 0;
        return;
    }

    L->tail->next = R->head;
    L->tail = R->tail;
    L->size += R->size;

    R->head = NULL;
    R->tail = NULL;
    R->size = 0;
}

static void board_list_destroy(void* key, void* value) {
    (void)key;
    BoardList* list = (BoardList*)value;

    Node* cur = list->head;
    while (cur != NULL) {
        Node* nxt = cur->next;
        free(cur);
        cur = nxt;
    }

    list->head = NULL;
    list->tail = NULL;
    list->size = 0;
}

typedef CILK_C_DECLARE_REDUCER(BoardList) BoardListReducer;

static BoardListReducer X = CILK_C_INIT_REDUCER(
    BoardList,
    board_list_reduce,
    board_list_identity,
    board_list_destroy,
    (BoardList){ .head = NULL, .tail = NULL, .size = 0 }
);

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
                       int depth, uint64_t sol) {
    if (row == ALL) {
        append_solution(&REDUCER_VIEW(X), sol);
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
                   sol | ((uint64_t)place << (depth * 8)));
    }
}

static void try(unsigned int row, unsigned int left, unsigned int right,
                int depth, uint64_t sol) {
    if (row == ALL) {
        append_solution(&REDUCER_VIEW(X), sol);
        return;
    }

    if (depth >= CUTOFF) {
        try_serial(row, left, right, depth, sol);
        return;
    }

    unsigned int poss = ~(row | left | right) & ALL;

    while (poss != 0) {
        unsigned int place = poss & -poss;
        poss &= ~place;

        cilk_spawn try(row | place,
                       (left | place) << 1,
                       (right | place) >> 1,
                       depth + 1,
                       sol | ((uint64_t)place << (depth * 8)));
    }

    cilk_sync;
}

int main(void) {
    CILK_C_REGISTER_REDUCER(X);

    try(0, 0, 0, 0, 0);

    printf("There are %d solutions.\n", X.value.size);

    // 如果你想打印所有解，取消注释：
    // for (Node* p = X.value.head; p != NULL; p = p->next) {
    //     print_solution(p->sol);
    // }

    CILK_C_UNREGISTER_REDUCER(X);
    return 0;
}