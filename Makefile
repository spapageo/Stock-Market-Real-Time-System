all: marketSim


strip: marketSim
	strip -s marketSim

marketSim: marketSim.c market.c stop.c limit.c marketSim.h market.h stop.h limit.h
	gcc -Wall -O3 -ffast-math marketSim.c market.c stop.c stoplimit.c limit.c -lpthread -o marketSim

clang: marketSim.c market.c stop.c limit.c marketSim.h market.h stop.h limit.h
	clang -Wall -O4 -ffast-math -funroll-loops marketSim.c market.c stop.c stoplimit.c limit.c -lpthread -o marketSim

debug: marketSim.c market.c stop.c limit.c marketSim.h market.h stop.h limit.h
	gcc -Wall -g3 -O3 marketSim.c market.c stop.c stoplimit.c limit.c -lpthread -o marketSim

profile: marketSim.c market.c stop.c limit.c marketSim.h market.h stop.h limit.h
	gcc -Wall -pg -g3 -O3 marketSim.c market.c stop.c stoplimit.c limit.c -lpthread -o marketSim

clean:
	rm -rf marketSim marketSim.exe logfile.txt *~
