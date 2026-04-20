#include <libgen.h>
#include <stdio.h>
#include <string.h>

#define MAXCELLS 18
#define STACKS 4
#define DISKS 14
#define RUNS 100

struct element {
    int discsize;
    int next;
};

static int stack_arr[STACKS];
static struct element cellspace[MAXCELLS + 1];
static int freelist;
static int movesdone;

static void make_null(int s) {
    stack_arr[s] = 0;
}

static int get_element(void) {
    if (freelist <= 0) {
        return 0;
    }
    int temp = freelist;
    freelist = cellspace[freelist].next;
    return temp;
}

static void push_disc(int disc, int s) {
    if (stack_arr[s] > 0 && cellspace[stack_arr[s]].discsize <= disc) {
        return;
    }
    int slot = get_element();
    if (slot == 0) {
        return;
    }
    cellspace[slot].next = stack_arr[s];
    stack_arr[s] = slot;
    cellspace[slot].discsize = disc;
}

static int pop_disc(int s) {
    if (stack_arr[s] <= 0) {
        return 0;
    }
    int top = stack_arr[s];
    int disc = cellspace[top].discsize;
    stack_arr[s] = cellspace[top].next;
    cellspace[top].next = freelist;
    freelist = top;
    return disc;
}

static void move_disc(int s1, int s2) {
    int disc = pop_disc(s1);
    if (disc != 0) {
        push_disc(disc, s2);
        movesdone += 1;
    }
}

__attribute__((section("OPTIMIZABLE_SECTION")))
static void tower(int src, int dst, int k) {
    if (k == 1) {
        move_disc(src, dst);
        return;
    }
    int aux = 6 - src - dst;
    tower(src, aux, k - 1);
    move_disc(src, dst);
    tower(aux, dst, k - 1);
}

__attribute__((section("OPTIMIZABLE_SECTION")))
static int run_towers_kernel(void) {
    for (int i = 1; i <= MAXCELLS; i++) {
        cellspace[i].next = i - 1;
    }
    freelist = MAXCELLS;

    make_null(1);
    make_null(2);
    make_null(3);
    for (int d = DISKS; d >= 1; d--) {
        push_disc(d, 1);
    }

    movesdone = 0;
    tower(1, 2, DISKS);
    return movesdone;
}

int main(void) {
    unsigned long long checksum = 0;
    for (int i = 0; i < RUNS; i++) {
        checksum += (unsigned long long)run_towers_kernel();
    }

    char filepath[512];
    strcpy(filepath, __FILE__);
    char *dir = dirname(filepath);
    char output_path[1024];
    snprintf(output_path, sizeof(output_path), "%s/output.txt", dir);

    FILE *file = fopen(output_path, "w");
    if (!file) {
        return 1;
    }
    fprintf(file, "%.0f\n", (double)checksum);
    fclose(file);

    printf("%.0f\n", (double)checksum);
    return 0;
}
