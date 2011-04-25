#! /usr/bin/env python

import os

for x in range(9):
	os.system("cp 0" + str(x+1) + "_*.jpg " + str(x+1) + ".jpg")
os.system("cp 10_*.jpg 10.jpg")

