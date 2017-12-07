#!/bin/env python
import re
import fileinput
for line in fileinput.input():
    print(re.findall('(my )?\$(\w+?)\W', line))
