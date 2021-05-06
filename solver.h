
enum move_t { U, D, F, B, L, R };

/*
Layout:
      2
    1   3
      0

9  8     11 10

      4
    5   7
      6
*/
char move_table_ep[6][12] = {
    {1,   2,  3,  0, -1, -1, -1, -1, -1, -1, -1, -1},
    {-1, -1, -1, -1,  7,  4,  5,  6, -1, -1, -1, -1},
    {11, -1, -1, -1,  8, -1, -1, -1,  0, -1, -1,  4},
    {-1, -1,  9, -1, -1, -1, 10, -1, -1,  4,  2, -1},
    {-1,  8, -1, -1, -1,  9, -1, -1,  5,  1, -1, -1},
    {-1, -1, -1, 10, -1, -1, -1, 11, -1, -1,  7,  3}
};
char move_table_eo[6][12] = {
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 1},
    {0, 0, 1, 0, 0, 0, 1, 0, 0, 1, 1, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
};
struct cube_t {
    //short eo;
    char eo[12];
    char ep[12];
};

struct zcube_t {
    int eo;
    char ep[4];
};

typedef struct zcube_t zcube_t;
typedef struct cube_t cube_t;
typedef enum move_t move_t;


cube_t mk_cube();

int is_solved(cube_t*);

void print_cube(cube_t*);

void apply(cube_t*, move_t, int);

int id_eocross(cube_t*);
