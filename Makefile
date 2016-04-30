all:
	gcc my_memory.c test4.c -o test4_out
	gcc my_memory.c test5.c -o test5_out

clean:
	rm test?_out test?_output.txt
