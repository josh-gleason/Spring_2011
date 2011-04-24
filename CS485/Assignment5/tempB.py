#! /usr/bin/env python

import os

def drange(start, stop, step):
	r = start
	while r < stop:
		yield r
		r += step

os.system("rm out.txt");
for x in drange(0.01,0.53,0.01):
	print x
	os.system("./testb traindata/train_L_b images/fb_L_images.txt " + str(x) + " >> out.txt")
