import unittest
from abc import abstractmethod, abstractclassmethod
from typing import List, Callable, Tuple, Union, Optional, Any
NoneType = type(None)
from math import inf
import time
import subprocess
import os
import numpy as np
from test_integration_base import TestIntegrationBase, random_blob, random_blob_symmetric, str_to_mat
import spkmeansref as spkmeansmodule_ref
import random
import inspect


class TestIntegrationGoodBase(TestIntegrationBase):
    """
    Testing class for good cases - valid inputs, in standalone C
    """

    path_to_repo_folder: str = "/home/fakename/workspace/softproj"
    path_to_writable_folder: str = "/tmp"

    def test_wam(self):
        blob = random_blob('tiny')
        result = str_to_mat(self.run_with_data('wam', blob))
        result_ref = spkmeansmodule_ref.full_wam(blob)
        self.assert_mat_dist(result_ref, result)
    
    def test_ddg(self):
        blob = random_blob('tiny')
        result = str_to_mat(self.run_with_data('ddg', blob))
        result_ref = spkmeansmodule_ref.full_ddg(blob)
        self.assert_mat_dist(result_ref, result)

    def test_lnorm(self):
        blob = random_blob('tiny')
        result = str_to_mat(self.run_with_data('lnorm', blob))
        result_ref = spkmeansmodule_ref.full_lnorm(blob)
        self.assert_mat_dist(result_ref, result)
    
    def test_jacobi(self):
        blob = random_blob_symmetric('tiny')
        result = str_to_mat(self.run_with_data('jacobi', blob))
        eigenvalues, eigenvectors = result[0], result[1:]
        eigenvalues_ref, eigenvectors_ref = spkmeansmodule_ref.full_jacobi(blob)
        self.assert_mat_dist(eigenvalues_ref, eigenvalues)
        self.assert_mat_dist(eigenvectors_ref, eigenvectors)
