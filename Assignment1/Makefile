.PHONY: all clean

all:	slow-pali fast-pali

slow-pali: slow-pali.cpp
	g++ -O2 -Wall slow-pali.cpp -o slow-pali

fast-pali: fast-pali.cpp
	g++ -O2 -Wall fast-pali.cpp -o fast-pali

clean:
	-/bin/rm -f slow-pali fast-pali *.o *~

test:	slow-pali fast-pali
	-@echo ------------------------------------------------------
	-@echo Testing on 1KB
	./dup.py 1000 < t4.txt | time -p ./slow-pali
	./dup.py 1000 < t4.txt | time -p ./fast-pali
	./dup.py 1000 < t4.txt | time -p python3 ./palindrome.py
	-@echo ------------------------------------------------------
	-@echo Testing on 1MB
	./dup.py 1000000 < t4.txt | time -p ./slow-pali
	./dup.py 1000000 < t4.txt | time -p ./fast-pali
	./dup.py 1000000 < t4.txt | time -p python3 ./palindrome.py
	-@echo ------------------------------------------------------
	-@echo Testing on 100MB
	./dup.py 100000000 < t4.txt | time -p ./fast-pali
	./dup.py 100000000 < t4.txt | time -p python3 ./palindrome.py
	-@echo ------------------------------------------------------
	-@echo Testing on 2GB
	./dup.py 2000000000 < t1.txt | time -p ./fast-pali
	./dup.py 2000000000 < t2.txt | time -p ./fast-pali
	./dup.py 2000000000 < t3.txt | time -p ./fast-pali
	./dup.py 2000000000 < t4.txt | time -p ./fast-pali
	./dup.py 2000000000 < t5.txt | time -p ./fast-pali
	./dup.py 2000000000 < t6.txt | time -p ./fast-pali
