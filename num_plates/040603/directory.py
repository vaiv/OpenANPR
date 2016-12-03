import os, glob, subprocess, sys


count=0
extensions = ("*.jpg","*.jpeg")
f = open('dir.sh', 'w')

for extension in extensions:
    for file in glob.glob("*"+extension):
	count=count+1
	path=' ./res/'+str(count)
	cmd = 'mkdir '+path
	f.write(cmd + '\n')
	

f.close()
