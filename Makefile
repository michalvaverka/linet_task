make: 
	gcc -Wall -Werror --pedantic -g ./main.c ./HDD_impl.c ./FS.c -o hdd_test
test:
	valgrind ./hdd_test