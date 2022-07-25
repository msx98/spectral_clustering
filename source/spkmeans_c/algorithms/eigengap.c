#include "eigengap.h"

status_t sort_cols_by_vector_desc(mat_t* A, mat_t* v) {
    real* eigenvalues;
    status_t status;
    uint* sorting_indices;
    uint n, i;
    assert(A);
    assert(v);
    assert(A->h == A->w);
    assert(v->h == v->w);
    assert(A->h == v->h);

    n = v->h;
    eigenvalues = malloc(sizeof(real)*n);
    if (!eigenvalues) return ERROR_MALLOC;
    for (i=0; i<n; i++) eigenvalues[i] = mat_get(v,i,i);
    sorting_indices = argsort_desc(eigenvalues, n);
    free(eigenvalues);

    status = sort_cols_by_vector(A, sorting_indices);
    if (status != SUCCESS) return status;
    status = sort_cols_by_vector(v, sorting_indices);
    if (status != SUCCESS) return status;
    return SUCCESS;
}

bool is_diagonal(mat_t* A) {
    return calc_off_squared(A) == 0;
}

bool is_square(mat_t* A) {
    return A->w == A->h;
}

uint calc_k(mat_t* eigenvalues) {
    uint i, n, half_n;
    uint max_eigengap_idx;
    real max_eigengap_val, eigengap_val;
    assert(is_square(eigenvalues));
    assert(is_diagonal(eigenvalues));
    n = eigenvalues->h;
    half_n = floor(((double)n)/2);
    assert(half_n >= 1);
    max_eigengap_idx = 0;
    max_eigengap_val = mat_get(eigenvalues,0,0);
    for (i=1; i<n; i++) {
        eigengap_val = mat_get(eigenvalues,i,i);
        if (eigengap_val > max_eigengap_val) {
            max_eigengap_idx = i;
            max_eigengap_val = eigengap_val;
        }
    }
    return max_eigengap_idx;
}