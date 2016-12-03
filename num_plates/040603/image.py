import os, glob, subprocess, sys

program = '../../ANPR'
count=0
extensions = ("*.jpg","*.jpeg")
f = open('convertor.sh', 'w')

for extension in extensions:
    for file in glob.glob("*"+extension):
	count=count+1
	path=' ./res/'+str(count)
        argument = [ program , ' ../../'+file]
	cmd2 = 'cd '+path
	cmd3 = 'cd ../../'
        print ' '.join(argument)
	f.write(cmd2 + '\n'+  ' '.join(argument) + '\n' + cmd3 + '\n')
	

f.close()
