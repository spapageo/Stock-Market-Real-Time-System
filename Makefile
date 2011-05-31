all:	marketSim.c
	gcc -O3 marketSim.c market.c -lpthread -o marketSim
 
