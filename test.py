#include

import py_compile
a = 1           
b = 2
print(a+b)
py_compile.compile('test.py')  # This will compile the test.py file