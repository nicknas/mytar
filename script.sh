#!/bin/bash
directorio_actual=$(pwd)
if  [ -f "$directorio_actual/mytar" ] &&  [ -x "$directorio_actual/mytar" ]
then
	if [ -e "$directorio_actual/tmp" ]
	then
		rm -r "$directorio_actual/tmp";
	fi
	mkdir "tmp";
	cd tmp;
	touch file1.txt;
	touch file2.txt;
	touch file3.dat;
	echo "Hello world!" > file1.txt;
	head "/etc/passwd" > file2.txt;
	head -c 1024 "/dev/urandom" > file3.dat;
	../mytar -cf filetar.mtar file1.txt file2.txt file3.dat;
	mkdir "out";
	cp filetar.mtar out;
	cd out;
	../../mytar -xf filetar.mtar;
	compare1=`diff file1.txt ../file1.txt`
	compare2=`diff file2.txt ../file2.txt`
	compare3=`diff file3.dat ../file3.dat`
	if [ -z $compare1 ] && [ -z $compare2 ] && [ -z $compare3 ]
	then
		echo "Correct"
		cd ../..;
		exit 0;
	else
		echo $compare1
		echo $compare2
		echo $compare3
		cd ../..;
		exit 1;
	fi 
else 
	echo "el ejecutable no existe en el directorio"
fi

