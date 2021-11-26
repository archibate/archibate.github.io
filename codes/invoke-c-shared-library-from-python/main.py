import ctypes
import sys
import os

def load_library(path, name):
    if sys.platform == 'win32':
        return ctypes.cdll.LoadLibrary(os.path.join(path, name + '.dll'))
    elif sys.platform == 'linux':
        return ctypes.cdll.LoadLibrary(os.path.join(path, 'lib' + name + '.so'))
    elif sys.platform == 'darwin':
        return ctypes.cdll.LoadLibrary(os.path.join(path, 'lib' + name + '.dylib'))
    else:
        raise ImportError('Unsupported platform: ' + sys.platform)

mylib = load_library('build', 'mylib')

mylib.twice_int.argtypes = [ctypes.c_int]
mylib.twice_int.restype = ctypes.c_int
mylib.twice_float.argtypes = [ctypes.c_float]
mylib.twice_float.restype = ctypes.c_float
mylib.print_str.argtypes = [ctypes.c_char_p]
mylib.print_str.restype = None

print(mylib.twice_int(21))       # 42
print(mylib.twice_float(3.14))   # 6.28
mylib.print_str(b'Hello, C++!')

mylib.test_array.argtypes = [ctypes.c_void_p, ctypes.c_size_t]
mylib.test_array.restype = None

import numpy as np

arr = np.random.rand(32).astype(np.float32)
mylib.test_array(arr.ctypes.data, arr.shape[0])
