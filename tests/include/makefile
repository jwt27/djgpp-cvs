all : testinc1.exe testinc2.exe

testinc1.exe : testinc1.o
	gcc $< -o $@

testinc2.exe : testinc2.o
	gcc $< -o $@

testinc1.o : testinc1.c
testinc2.o : testinc2.c
