#include <stdio.h>

void try(int row, int left, int right);

static int count = 0;

int main() {
    try(0, 0, 0);
    printf("There are %d solutions.\n", count);
    return 0;
}

void try(int row, int left, int right) {
    int poss, place;

    if (row == 0xFF) {
        ++count;
    } else {
        poss = ~(row | left | right) & 0xFF;
        while (poss != 0) {
            place = poss & -poss;
            try(row | place,
                (left | place) << 1,
                (right | place) >> 1);
            poss &= ~place;
        }
    }
}