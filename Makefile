all: 
	gcc -o cpu_multi_code cpu_multi_code.c

clean:
	rm -f *.o cpu_multi_code

run:
	./cpu_multi_code < code.bin

zip:
	@zip -r cpu_multi_code_T1_Grupo08 Makefile Leia-Me.txt cpu_multi_code.c code.bin