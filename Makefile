out: out.o
	ld -o out out.o -lSystem -syslibroot `xcrun -sdk macosx --show-sdk-path` -e _start -arch arm64 

out.o: out.s
	as -arch arm64 -o out.o out.s
