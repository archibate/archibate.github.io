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

mylib.say_hello()

print(mylib.twice_int(32))

s = b'hello'
mylib.print_str(ctypes.c_char_p(s))

s = '我系佳佳辉'.encode()
mylib.print_str(ctypes.c_char_p(s))

import numpy as np

x = np.random.rand(16).astype(np.float32)
mylib.test_array(ctypes.c_void_p(x.ctypes.data), x.shape[0])
