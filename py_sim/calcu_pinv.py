#! /usr/bin/env python3

import numpy as np


dt=0.1

if __name__ == "__main__":
    a=np.arange(12).reshape([4,3])
    print(a)
    b=np.arange(12)
    print(b)
    for i in b:
        print(b.get(i))
