#include <stdio.h>
#include <time.h>
#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include "solver.h"


cube_t mk_cube() {
    cube_t cube;
    for (int i = 0; i < 12; i++) {
        cube.eo[i] = 0;
    }
    for (int i = 0; i < 12; i++) {
        cube.ep[i] = i;
    }
    return cube;
}

int is_solved(cube_t* cube) {
    for (int i = 0; i < 12; i++) {
        if (cube->eo[i]) return 0;
    }
    for (int i = 0; i < 12; i++) {
        if (cube->ep[i] != i) return 0;
    }
    return 1;
}

void print_cube(cube_t* cube) {
    printf("eo: ");
    for (int i = 0; i < 12; i++) {
        printf("%d ", cube->eo[i]);
    }
    printf(", ep: ");
    for (int i = 0; i < 12; i++) {
        printf("%d ", cube->ep[i]);
    }
    printf("\n"); 
}

void apply(cube_t* cube, move_t move, int ntimes) {
    cube_t cube1 = mk_cube();
    char* table_ep = move_table_ep[move];
    char* table_eo = move_table_eo[move];

    // TODO: load using bit op
    for (int i = 0; i < ntimes; i++) {
        for (int src = 0; src < 12; src++) {
            int dst = table_ep[src];
            if (dst != -1) {
                cube1.ep[dst] = cube->ep[src];
                cube1.eo[dst] = cube->eo[src] ^ table_eo[src];
                //eo1 &= ~(1 << dst);
                // eo1 |= ( ( (cube->eo >> src) & 1)  ^ table_eo[src]) << dst;
            }
        }
        for (int src = 0; src < 12; src++) {
            if (table_ep[src] != -1) {
                cube->ep[src] = cube1.ep[src];
                cube->eo[src] = cube1.eo[src];
            }
        }
    }
}

void apply_z(zcube_t* cube, move_t move, int ntimes) {
    zcube_t cube1;
    char* table_ep = move_table_ep[move];
    char* table_eo = move_table_eo[move];

    // TODO: load using bit op
    for (int i = 0; i < ntimes; i++) {
        for (int src = 0; src < 12; src++) {
            int dst = table_ep[src];
            if (dst != -1) {
                cube1.ep[dst] = cube->ep[src];
                //cube1.eo[dst] = cube->eo[src] ^ table_eo[src];
            }
        }
        for (int src = 0; src < 12; src++) {
            if (table_ep[src] != -1) {
                cube->ep[src] = cube1.ep[src];
                //cube->eo[src] = cube1.eo[src];
            }
        }
    }
}

int id_eocross(cube_t* cube) {
    // find where the cross edges are (4,5,6,7), compute their index
    int pos[4] = {};
    char* eps = cube->ep;
    char* eos = cube->eo;
    for (int i = 0; i < 12; i++) {
        char ep = eps[i];
        switch (ep) {
            case 4: pos[0] = i; break;
            case 5: pos[1] = i; break;
            case 6: pos[2] = i; break;
            case 7: pos[3] = i; break;
            default: break;
        }
    }
    int eo = 0;
    for (int i = 0; i < 11; i++) {
        eo |= eos[i];
        eo <<= 1;
    }

    int posidx = pos[0] * (12*12*12) + pos[1] * (12*12) + pos[2] * 12 + pos[3];
    int idx = posidx * (1 << 11) + eo;
    return idx;
}

void measure_perf() {
    #define N_RUNS 1000000
    int volatile hash = 0;
    cube_t cube = mk_cube();
    clock_t start;
    double elapsed;

    start = clock();

    for (int i = 0; i < N_RUNS; i++) {
        hash |= id_eocross(&cube);
    }
    elapsed = (double)(clock() - start) / CLOCKS_PER_SEC;
    printf("id_eocross on %d runs = %lf s\n", N_RUNS, elapsed);

    start = clock();
    for (int i = 0; i < N_RUNS / 3; i++) {
        apply(&cube, R, 1);
        apply(&cube, U, 1);
        apply(&cube, F, 1);
    }
    elapsed = (double)(clock() - start) / CLOCKS_PER_SEC;
    printf("apply on %d runs = %lf s\n", N_RUNS, elapsed);
}

void test() {
    cube_t cube = mk_cube();
    int id_solved = id_eocross(&cube);
    for (int i = 0; i < 6; i++) {
        apply(&cube, R, 1);
        apply(&cube, U, 1);
        apply(&cube, R, 3);
        apply(&cube, U, 3);
    }
    int id_scr = id_eocross(&cube);
    assert(id_solved == id_scr);
    assert(is_solved(&cube));
    print_cube(&cube);

    for (int i = 0; i < 6; i++) {
        move_t move = (move_t) i;
        apply(&cube, move, 1);
        //printf("id = %d\n", id_eocross(&cube));
        assert(!is_solved(&cube));
        apply(&cube, move, 3);
        print_cube(&cube);
        assert(is_solved(&cube));
    }
    printf("id_solved = %d\n", id_solved);

    printf("All tests passed.\n");
}

char* gen_table(cube_t solved, int depth, int size) {
#define MAX_TABLE_SIZE 43000000 // 12^4 * 2048
    assert(size < MAX_TABLE_SIZE);
    char* table = (char*) malloc(size * sizeof(char));
    memset(table, -1, size * sizeof(char));

#define MAX_LEVEL_SIZE 1000000
    cube_t* q1 = (cube_t*) malloc(MAX_LEVEL_SIZE * sizeof(cube_t)); 
#define MAX_LEVEL_SIZE 1000000
    cube_t* q2 = (cube_t*) malloc(MAX_LEVEL_SIZE * sizeof(cube_t)); 

    cube_t* qcurr = q1;
    cube_t* qnext = q2;

    int qc_len = 1, qn_len = 0;
    // 24 bytes, if we wanna store everything that takes ~600M!
    qcurr[0] = solved;

    int tot_len = 1;
    for (int d = 1; d <= depth; d++) {
        for (int i = 0; i < qc_len; i++) {
            cube_t cube = qcurr[i];
            for (int j = 0; j < 6; j++) {
                for (int k = 0; k < 3; k++) {
                    apply(&cube, j, 1);
                    // try place
                    int id = id_eocross(&cube);
                    if (table[id] == -1) {
                        table[id] = d;
                        qnext[qn_len++] = cube;
                    }
                }
            }
        }
        printf("depth %d : %d states\n", d, qn_len);
        tot_len += qn_len;
        cube_t* tmp = qcurr; qcurr = qnext; qnext = tmp;
        qc_len = qn_len; qn_len = 0;
    }

    printf("%d states in total \n", tot_len);

    free(q1);
    free(q2);
    return table;
}

void solve(cube_t cube, int id_solved){
}

int main() {
    test();
    measure_perf();

    gen_table(mk_cube(), 5, 12*12*12*12*2048);
    return 0;
}
