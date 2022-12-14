
#include "matrix.h"


/* initialize mat(h,w) with zero and return */
mat_t* mat_init(const uint h, const uint w) {
    mat_t* mat;
    int mat_size;
    mat_size = h*w;
    mat = malloc(sizeof(mat_t));
    if (!mat) return NULL;
    mat->__swap_axes = false;
    mat->h = h;
    mat->w = w;
    if (mat_size > 0) {
        mat->__data = calloc(mat_size, sizeof(real));
        if (!mat->__data) {
            free(mat);
            return NULL;
        }
    } else {
        mat->__data = NULL;
    }
    return mat;
}

/* initialize mat(h,w) and fill with value */
mat_t* mat_init_full(const uint h, const uint w, const real value) {
    mat_t* mat = mat_init(h,w);
    if (!mat) return NULL;
    mat_mul_scalar(mat, mat, 0);
    mat_add_scalar(mat, mat, value);
    return mat;
}

/* initialize mat(n,n) with zeros, fill diagonal with 1 */
mat_t* mat_init_identity(const uint n) {
    uint i;
    mat_t* mat = mat_init_full(n,n,0);
    if (!mat) return NULL;
    for (i=0; i<n; i++) mat_set(mat,i,i,1);
    return mat;
}

void mat_free(mat_t** mat) {
    free((*mat)->__data);
    free(*mat);
}

/* transpose mat */
void mat_transpose(mat_t* mat) {
    swap_uint(&mat->h, &mat->w);
    mat->__swap_axes = !mat->__swap_axes;
}

/* return mat[i][j] if __transposed==0, else mat[j][i] */
real mat_get(mat_t* mat, uint i, uint j) {
    real value;
    if (mat->__swap_axes) {
        /* not thread safe but whatever lol */
        mat_transpose(mat);
        value = mat_get(mat, j, i);
        mat_transpose(mat);
        return value;
    }

    if ((i >= mat->h) || (j >= mat->w)) {
        printd(("mat_get: Attempted to access invalid matrix indices: %d >= %d or %d >= %d\n", i, mat->h, j, mat->w));
        #ifdef FLAG_DEBUG
        exit(1);
        #endif
    }
    return mat->__data[(i*mat->w)+j];
}

/* mat[i][j] = new_value if __transposed==0, else mat[j][i] */
void mat_set(mat_t* mat, uint i, uint j, const real new_value) {
    if (mat->__swap_axes) {
        /* not thread safe but whatever lol */
        mat_transpose(mat);
        mat_set(mat, j, i, new_value);
        mat_transpose(mat);
        return;
    }

    if ((i >= mat->h) || (j >= mat->w)) {
        printd(("mat_set: Attempted to access invalid matrix indices: %d >= %d or %d >= %d\n", i, mat->h, j, mat->w));
        #ifdef FLAG_DEBUG
        exit(1);
        #endif
    }
    mat->__data[(i*mat->w)+j] = new_value;
}

/* return a copy of mat */
void mat_copy_to(mat_t* dst, mat_t* src) {
    uint i, j, h, w;
    assertd(dst); assertd(src);
    assertd_same_dims(dst, src);
    h = dst->h;
    w = dst->w;
    for (i=0; i<h; i++) {
        for (j=0; j<w; j++) {
            mat_set(dst, i, j, mat_get(src, i, j));
        }
    }
}


/* return a copy of mat */
mat_t* mat_init_copy(mat_t* mat) {
    mat_t* copy = mat_init(mat->h, mat->w);
    if (!copy) return NULL;
    mat_copy_to(copy, mat);
    return copy;
}

/* return mat with same dims as mat */
mat_t* mat_init_like(mat_t* mat) {
    mat_t* copy = mat_init(mat->h, mat->w);
    if (!copy) return NULL;
    return copy;
}

void __mat_cellwise(mat_t* dst, mat_t* mat_1, mat_t* mat_2, real (*op)(real, real)) {
    uint i, j;
    assertd((mat_1->h == mat_2->h) && (mat_2->h == dst->h));
    assertd((mat_1->w == mat_2->w) && (mat_2->w == dst->w));
    for (i=0; i<dst->h; i++) {
        for (j=0; j<dst->w; j++) {
            mat_set(dst, i, j, op(mat_get(mat_1, i, j),mat_get(mat_2, i, j)));
        }
    }
}

void __mat_cellwise_scalar(mat_t* dst, mat_t* mat, const real alpha, real (*op)(real, real)) {
    uint i, j;
    assertd(mat->h == dst->h);
    assertd(mat->w == dst->w);
    for (i=0; i<dst->h; i++) {
        for (j=0; j<dst->w; j++) {
            mat_set(dst, i, j, op(mat_get(mat, i, j), alpha));
        }
    }
}

/* dst = mat_1 + mat_2 */
void mat_add_cellwise(mat_t* dst, mat_t* mat_1, mat_t* mat_2) {
    __mat_cellwise(dst, mat_1, mat_2, real_add);
}

/* dst = mat_1 - mat_2 */
void mat_sub_cellwise(mat_t* dst, mat_t* mat_1, mat_t* mat_2) {
    __mat_cellwise(dst, mat_1, mat_2, real_sub);
}

/* dst = mat_1 * mat_2 */
void mat_mul_cellwise(mat_t* dst, mat_t* mat_1, mat_t* mat_2) {
    __mat_cellwise(dst, mat_1, mat_2, real_mul);
}

/* dst = mat_1 / mat_2 */
void mat_div_cellwise(mat_t* dst, mat_t* mat_1, mat_t* mat_2) {
    __mat_cellwise(dst, mat_1, mat_2, real_div);
}

/* dst = mat_1 ^ mat_2 */
void mat_pow_cellwise(mat_t* dst, mat_t* mat_1, mat_t* mat_2) {
    __mat_cellwise(dst, mat_1, mat_2, real_pow);
}

/* dst = mat + alpha */
void mat_add_scalar(mat_t* dst, mat_t* mat, const real alpha) {
    __mat_cellwise_scalar(dst, mat, alpha, real_add);
}

/* dst = mat - alpha */
void mat_sub_scalar(mat_t* dst, mat_t* mat, const real alpha) {
    __mat_cellwise_scalar(dst, mat, alpha, real_sub);
}

/* dst = mat * alpha */
void mat_mul_scalar(mat_t* dst, mat_t* mat, const real alpha) {
    __mat_cellwise_scalar(dst, mat, alpha, real_mul);
}

/* dst = mat / alpha */
void mat_div_scalar(mat_t* dst, mat_t* mat, const real alpha) {
    __mat_cellwise_scalar(dst, mat, alpha, real_div);
}

/* dst = mat ^ alpha */
void mat_pow_scalar(mat_t* dst, mat_t* mat, const real alpha) {
    __mat_cellwise_scalar(dst, mat, alpha, real_pow);
}

/* dst = alpha ^ mat */
void mat_scalar_pow(mat_t* dst, mat_t* mat, const real alpha) {
    __mat_cellwise_scalar(dst, mat, alpha, real_pow_rev);
}

/* dst = dst @ src */
void mat_mul(mat_t* dst, mat_t* mat_lhs, mat_t* mat_rhs) {
    uint i, j, k;
    real value;
    real val_lhs, val_rhs;
    assertd(dst != mat_lhs); assertd(dst != mat_rhs);
    assertd(mat_lhs->w == mat_rhs->h);
    assertd(dst->w == mat_rhs->w);
    assertd(dst->h == mat_lhs->h);
    for (i=0; i<dst->h; i++) {
        for (j=0; j<dst->w; j++) {
            value = 0;
            for (k=0; k<mat_lhs->w; k++) {
                /*printd(("getting %d, %d, %d\n", i, j, k));*/
                val_lhs = mat_get(mat_lhs, i, k);
                val_rhs = mat_get(mat_rhs, k, j);
                value += val_lhs*val_rhs;
            }
            mat_set(dst, i, j, value);
        }
    }
}

mat_t* matmul(mat_t* mat_lhs, mat_t* mat_rhs) {
    mat_t* dst;
    dst = mat_init(mat_lhs->h, mat_rhs->w);
    if (!dst) return NULL;
    mat_mul(dst, mat_lhs, mat_rhs);
    return dst;
}

void mat_swap_cols(mat_t* A, const uint col_1, const uint col_2) {
    uint row;
    real tmp;
    for (row=0; row<A->h; row++) {
        tmp = mat_get(A, row, col_1);
        mat_set(A, row, col_1, mat_get(A, row, col_2));
        mat_set(A, row, col_2, tmp);
    }
}

void mat_normalize_rows(mat_t* dst, mat_t* src) {
    uint i, j, n, k;
    real uij, sum_j_uij_2;
    assertd_same_dims(dst, src);
    n = dst->h;
    k = dst->w;

    for (i=0; i<n; i++) {
        sum_j_uij_2 = 0;
        for (j=0; j<k; j++) {
            uij = mat_get(src, i, j);
            sum_j_uij_2 += uij*uij;
        }
        sum_j_uij_2 = real_pow(sum_j_uij_2,(real)0.5);
        if (sum_j_uij_2 == 0) continue;
        for (j=0; j<k; j++) {
            uij = mat_get(src, i, j);
            mat_set(dst, i, j, uij/sum_j_uij_2);
        }
    }
}

status_t reorder_mat_cols_by_indices(mat_t* v, uint* indices) {
    uint i,j;
    mat_t* tmp;
    tmp = mat_init_like(v);
    if (!tmp) return ERROR_MALLOC;

    for (j=0; j<tmp->w; j++) {
        for (i=0; i<tmp->h; i++) {
            mat_set(tmp, i, j, mat_get(v, i, indices[j]));
        }
    }

    mat_copy_to(v, tmp);
    mat_free(&tmp);
    
    return SUCCESS;
}

real calc_off_squared(mat_t* A) {
    uint i, j, n;
    real off;
    assertd((A->h) == (A->w));
    n = A->h;
    off = 0;
    for (i=0; i<n; i++) {
        for (j=0; j<n; j++) {
            if (i!=j)
                /*off += real_pow(mat_get(A,i,j),(real)2.0);*/
                off += mat_get(A,i,j)*mat_get(A,i,j);
        }
    }
    return off;
}

bool is_diagonal(mat_t* A) {
    return calc_off_squared(A) == 0;
}

bool is_square(mat_t* A) {
    return A->w == A->h;
}

void mat_print(mat_t* mat) {
    uint i, j;
    assertd(mat);
    assertd((mat->w > 0) || ((mat->w == 0) && (mat->h==0)));
    for (i=0; i<mat->h; i++) {
        for (j=0; j<mat->w; j++) {
            printf("%.4f", mat_get(mat, i, j));
            if ((j+1) < mat->w) printf(",");
        }
        printf("\n");
    }
}

void mat_print_full(mat_t* mat) {
    uint i, j;
    assertd(mat);
    assertd((mat->w > 0) || ((mat->w == 0) && (mat->h==0)));
    for (i=0; i<mat->h; i++) {
        for (j=0; j<mat->w; j++) {
            printf("%f", mat_get(mat, i, j));
            if ((j+1) < mat->w) printf(",");
        }
        printf("\n");
    }
}

void mat_print_diagonal(mat_t* mat) {
    uint i, n;
    assertd(mat);
    assertd((mat->w > 0) || ((mat->w == 0) && (mat->h==0)));
    n = (mat->h > mat->w) ? mat->h : mat->w; /* min(h,w) */
    for (i=0; i<n; i++) {
        printf("%.4f", mat_get(mat, i, i));
        if ((i+1) < n) printf(",");
    }
    printf("\n");
}
