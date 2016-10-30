if [ ! -e $1/swigwin-1.3.39/swig.exe ] 
then 
	cwd=`pwd`;	cd $1; unzip swigwin-1.3.39.zip ; cd $cwd
fi
echo $1/swigwin-1.3.39/swig.exe $2 $3 $4 $5 $6 $7 $8 $9
$1/swigwin-1.3.39/swig.exe $2 $3 $4 $5 $6 $7 $8 $9
