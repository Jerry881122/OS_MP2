make distclean
make 
timeout 1 ../build.linux/nachos -e consoleIO_test1 -e consoleIO_test2
# timeout 1 ../build.linux/nachos -e consoleIO_test1
echo "done"
