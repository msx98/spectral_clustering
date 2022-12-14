import os
import sys
from typing import List
import numpy as np
import pandas as pd
import spkmeansmodule
import mykmeanssp
from enum import Enum
from typing import List, Union, Optional
import numpy as np
NoReturn = None # this is what I get for being on the latest version...

FLAG_DEBUG = False or bool(int(os.environ.get('FLAG_DEBUG','0')))
FLAG_VERBOSE_PRINTS = True and FLAG_DEBUG
FLAG_VERBOSE_ERRORS = True and FLAG_DEBUG
FLAG_EXIT_GRACEFULLY = False or not FLAG_DEBUG

MSG_ERR_INVALID_INPUT = "Invalid Input!"
MSG_ERR_GENERIC = "An Error Has Occurred"

INFINITY = float('inf')
JACOBI_MAX_ROTATIONS = 100
JACOBI_EPSILON = 1e-5
KMEANS_EPSILON = 0
KMEANS_MAX_ITER = 300 # this was verified to be 300

class InvalidInputTrigger(ValueError): pass
class GenericErrorTrigger(Exception): pass

def assertd(condition:bool) -> Union[None, NoReturn]:
    assert(condition)

def sign(num: int) -> int:
    assertd(num != np.nan)
    if num == 0:
        return 1
    else:
        return np.sign(num)


def main():
    np.random.seed(0)
    k, goal, datapoints = get_data_from_cmd()
    results = get_results(k, goal, datapoints)
    assertd(results)
    print('\n'.join([','.join(["%.4f"%y for y in x]) for x in results]))


def get_results(k: Optional[int], goal: str, datapoints: List[List[float]]) -> Union[List[List[float]],NoReturn]:
    if goal == 'spk':
        #L_norm = spkmeansmodule.full_lnorm(datapoints)
        #eigenvalues, eigenvectors = spkmeansmodule.full_jacobi_sorted(L_norm)
        #k = k or spkmeansmodule.full_calc_k(eigenvalues)
        #U = [x[:k] for x in eigenvectors]
        #T = spkmeansmodule.normalize_matrix_by_rows(U)
        if not k: k=0
        #print(f"Aboutta pass k=={k}")
        T = spkmeansmodule.full_spk_1_to_5(datapoints, k)
        k = k or len(T[0])
        results = calc_kmeanspp(k, T)
        print(','.join([str(x) for x in results[0]]))
        results = results[1:]
    elif goal == 'wam':
        results = spkmeansmodule.full_wam(datapoints)
    elif goal == 'ddg':
        results = spkmeansmodule.full_ddg(datapoints)
    elif goal == 'lnorm':
        results = spkmeansmodule.full_lnorm(datapoints)
    elif goal == 'jacobi':
        symmetric_matrix = datapoints # not actually datapoints
        #assertd(symmetric_matrix.shape[0] == symmetric_matrix.shape[1]) # should've been checked
        eigenvalues, eigenvectors = spkmeansmodule.full_jacobi(symmetric_matrix)
        results = [eigenvalues] + eigenvectors
    else:
        raise InvalidInputTrigger("Invalid goal specified!")
    return results


def calc_kmeanspp(k, datapoints):
    np.random.seed(0)
    fit_params = extract_fit_params(k, KMEANS_MAX_ITER, KMEANS_EPSILON, datapoints)
    results = KmeansAlgorithm(*fit_params)
    initial_centroids_indices_as_written = [int(fit_params[0][i]) for i in range(len(fit_params[0]))]
    return [initial_centroids_indices_as_written] + results


def extract_fit_params(*data_from_cmd, should_print=True):
    k, max_iter, eps, datapoints_list = data_from_cmd
    datapoints_list = np.array(datapoints_list)
    initial_centroids_indices = KMeansPlusPlus(k, datapoints_list)
    #initial_centroids_indices = select_actual_centroids(datapoints_list, initial_centroids_list)
    point_count, dims_count = datapoints_list.shape
    datapoints_list = datapoints_list.tolist()
    return (
        initial_centroids_indices,
        datapoints_list,
        dims_count,
        k,
        point_count,
        max_iter,
        eps
    )


def KMeansPlusPlus(k: int, x: np.array) -> List[int]:
    np.random.seed(0)
    x = np.array(x)
    N, d = x.shape
    u = [None for _ in range(k)]
    u_idx = [-1 for _ in range(N)]
    P = [0 for _ in range(N)]
    D = [float('inf') for _ in range(N)]

    #x_indices = np.arange(x.shape[0]).reshape(x.shape[0],1)
    #x = np.hstack((x_indices,x))
    n = x.shape[0]

    i = 0
    selection = np.random.choice(n)
    u[0] = x[selection]
    sels = [selection]

    while (i+1) < k:
        for l in range(N):
            x_l = x[l]
            min_square_dist = float('inf')
            for j in range(0,i+1):
                u_j = u[j]
                square_dist = np.sum((x_l - u_j)**2) # first item is an index
                min_square_dist = min(square_dist, min_square_dist)
            D[l] = min_square_dist
        D_sum = sum(D)
        if D_sum <= 0:
            raise Exception(f"Somehow reached D_sum = {D_sum}, but P values must be nonnegative and finite")
        P = D/D_sum

        i += 1
        selection = np.random.choice(n, p=P)
        u[i] = x[selection]
        sels.append(selection)
        continue
    #centroids_without_padding = [a[0] for a in u]
    return sels


def select_actual_centroids(data: List[List[float]], initial_centroids_list: List[List[float]]) -> List[int]:
    # incase we have duplicates, etc...
    initial_centroids_indices_actual = [None for centroid in initial_centroids_list]
    for i, centroid in enumerate(initial_centroids_list):
        loc = np.where(np.all(data==centroid,axis=1))[0] #[0] because this returns a tuple
        if len(loc) == 0: # or len(loc)>=2?
            raise GenericErrorTrigger(f"There should be exactly one match among the datapoints for every initial centroid, but one got {len(loc)} matches")
        initial_centroids_indices_actual[i] = loc[0]
    return initial_centroids_indices_actual


def KmeansAlgorithm(*fit_params) -> List[List[float]]:
    return mykmeanssp.fit(*fit_params)


def get_data_from_cmd():
    def _get_raw_cmd_args():
        args = sys.argv
        if not args:
            raise InvalidInputTrigger("Args are empty!")
        if args[0] in ["python", "python3", "python.exe", "python3.exe"]:
            args = args[1:]
            if not args:
                raise InvalidInputTrigger("Args are empty after removing \"python\" prefix")
        if args[0][-3:] == ".py":
            args = args[1:]
            if not args:
                raise InvalidInputTrigger("Args are empty after removing executable filename prefix")
        return args

    def _get_cmd_args():
        args = _get_raw_cmd_args()
        try:
            if len(args) == 2: # N/A, goal, file_name
                return None, args[0], args[1]
            if len(args) == 3: # k, goal, file_name
                return int(args[0]), args[1], args[2]
            else:
                raise InvalidInputTrigger("Cannot parse input format - number of args must be in {3}")
        except:
            raise InvalidInputTrigger("An error has occurred while parsing CLI input - one of the arguments is NaN")

    def _validate_input_filename(file: str):
        if not os.path.exists(file):
            raise InvalidInputTrigger(f"Specified path does not exist - \"{file}\"")
        if not (file.lower().endswith("csv") or file.lower().endswith("txt")):
            raise InvalidInputTrigger(f"Specified path does not end in a permitted extension - \"{file}\"")
    
    def _read_data_as_np(file_name: str) -> np.ndarray:
        _validate_input_filename(file_name)
        path_file = os.path.join(os.getcwd(), file_name)
        df = pd.read_csv(path_file, header=None)
        data = df.to_numpy()
        return data, path_file

    def _verify_params_make_sense(k: int, goal: str, data: np.ndarray):
        assertd(data.ndim == 2)
        n, d = data.shape
        if n == 0:
            raise GenericErrorTrigger("Data, as parsed, is empty - nothing to work on")
        if goal not in {'spk', 'wam', 'ddg', 'lnorm', 'jacobi'}:
            raise InvalidInputTrigger(f"Unrecognized goal specified - {goal}")
        if goal in {'spk'}:
            if k == None:
                raise InvalidInputTrigger(f"Only 2 parameters specified, expected integer k for goal {goal}")
            if not (0 <= k < n):
                raise InvalidInputTrigger("The following must hold: 0 <= k < n, but k={k} and n={n}")
            if (d - 1) < 1:
                raise GenericErrorTrigger("Datapoints number of dimensions must be at least 1, not including dimension 0 (index)")
            if any([(not first_indice.is_integer()) or (not (0 <= int(first_indice) < n)) \
                    for first_indice in data[:,0]]):
                #raise GenericErrorTrigger(f"One of the datapoints is missing a valid index in its first dimension")
                pass
        elif goal in {'wam', 'ddg', 'lnorm'}:
            if k != None:
                #raise InvalidInputTrigger(f"3 parameters specified, did not expect k for goal {goal}")
                pass
            if d < 1:
                raise GenericErrorTrigger("Datapoints number of dimensions must be at least 1")
        elif goal in {'jacobi'}:
            if k != None:
                #raise InvalidInputTrigger(f"3 parameters specified, did not expect k for goal {goal}")
                pass
            if d != n:
                raise GenericErrorTrigger(f"Jacobi expects a symmetric matrix, but n != d ({n} != {d})")
    
    k, goal, file_name = _get_cmd_args()
    datapoints, path_file = _read_data_as_np(file_name)
    _verify_params_make_sense(k, goal, datapoints)
    return k, goal, path_file


def exit_gracefully_with_err(err: Exception):
    def exit_gracefully_with_err_string(msg: str):
        print(msg)
        exit(1)
    error_to_string_map = {} if FLAG_VERBOSE_ERRORS else {
        InvalidInputTrigger: MSG_ERR_INVALID_INPUT       ,
        GenericErrorTrigger: MSG_ERR_GENERIC             }
    exit_gracefully_with_err_string(error_to_string_map.get(type(err),
        f"Exiting gracefully with an unexplained error:\n{str(err)}" if FLAG_VERBOSE_ERRORS \
        else MSG_ERR_GENERIC))


if __name__ == '__main__':
    if FLAG_EXIT_GRACEFULLY:
        try:
            main()
        except Exception as e:
            exit_gracefully_with_err(e)
    else:
        main()
