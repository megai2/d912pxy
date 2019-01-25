echo "Script for building:"
echo " - P7 library"
echo " - Examples"
echo " - Speed test"
echo " - Tracing tests"
echo "------------------------------------------------------------"

if [ $# -lt 2 ] ; then
   echo "run [./build.sh clean] to make cleanup"
fi

echo "P7----------------------------------------------------------"
cd ./Sources/
make $1
cd ..

cd ./Tests/

echo "Speed-------------------------------------------------------"
cd ./Speed/
make $1
cd ..

echo "Trace-------------------------------------------------------"
cd ./Trace/
make $1
cd ..

cd ..

cd ./Examples/

echo "Cpp Example-------------------------------------------------"
cd ./Cpp/
make $1

cd ..

echo "C Example-------------------------------------------------"
cd ./C/
make $1

cd ..

cd ..
