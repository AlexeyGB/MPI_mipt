#game_of_life_parallel: game_of_life_parallel.out

#game_of_life: game_of_life.out

game_of_life_parallel.out: game_of_life_parallel.o lib/life.o
	mpicc game_of_life_parallel.o lib/life.o -o game_of_life_parallel.out

game_of_life_parallel.o: game_of_life_parallel.c
	mpicc -c game_of_life_parallel.c -o game_of_life_parallel.o

lib/life.o: lib/life.c
	gcc -c lib/life.c -o lib/life.o

lib/life.o game_of_life.o: lib/life.h

game_of_life.out: game_of_life.o lib/life.o
	gcc game_of_life.o lib/life.o -o game_of_life.out

game_of_life.o: game_of_life.c
	gcc -c game_of_life.c -o game_of_life.o


clean:
	rm -rf *.out *.o lib/*.out lib/*.o

.PHONY: all clean
