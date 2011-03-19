#! /usr/bin/env python

import sys
import shlex, subprocess

newargs = [None]*(len(sys.argv)-1)
for x in range(len(sys.argv)-1):	
	newargs[x]=sys.argv[x+1];
p = subprocess.Popen(newargs)
