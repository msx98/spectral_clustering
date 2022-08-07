#include "eigengap.h"

status_t sort_cols_by_vector_desc(mat_t* A, mat_t* v) {
    real* eigenvalues;
    status_t status;
    uint* sorting_indices;
    uint n, i;

    assertd_is_square(A);
    assertd_same_dims(A, v);

    sorting_indices = NULL;
    eigenvalues = NULL;
    n = v->h;

    eigenvalues = malloc(sizeof(real)*n);
    if (!eigenvalues) {
        status = ERROR_MALLOC;
        goto sort_cols_by_vector_desc_finish;
    }
    for (i=0; i<n; i++) eigenvalues[i] = mat_get(v,i,i);

    sorting_indices = argsort_desc(eigenvalues, n);
    if (!sorting_indices) {
        status = ERROR_MALLOC;
        goto sort_cols_by_vector_desc_finish;
    }

    status = reorder_mat_cols_by_indices(A, sorting_indices);
    if (status != SUCCESS) goto sort_cols_by_vector_desc_finish;
    status = reorder_mat_cols_by_indices(v, sorting_indices);
    if (status != SUCCESS) goto sort_cols_by_vector_desc_finish;

    sort_cols_by_vector_desc_finish:
    if (sorting_indices) free(sorting_indices);
    if (eigenvalues) free(eigenvalues);
    return status;
}

uint calc_k(mat_t* eigenvalues) {
    uint i, n, half_n;
    uint max_eigengap_idx;
    real max_eigengap_val, eigengap_val;
    assertd(is_square(eigenvalues));
    assertd(is_diagonal(eigenvalues));
    n = eigenvalues->h;
    half_n = floor(((double)n)/2);
    assertd(half_n >= 1);
    max_eigengap_idx = 0;
    max_eigengap_val = mat_get(eigenvalues,0,0);
    for (i=1; i<half_n; i++) {
        eigengap_val = mat_get(eigenvalues,i,i);
        if (eigengap_val > max_eigengap_val) {
            max_eigengap_idx = i;
            max_eigengap_val = eigengap_val;
        }
    }
    return max_eigengap_idx;
}