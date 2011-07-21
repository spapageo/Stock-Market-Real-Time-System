all: strip


strip: marketSim
	strip -s marketSim

marketSim: marketSim.c market.c stop.c limit.c marketSim.h market.h stop.h limit.h
	gcc -Wall -mtune=native -march=native -O3 marketSim.c market.c stop.c limit.c -lpthread -o marketSim

debug: marketSim.c market.c stop.c limit.c marketSim.h market.h stop.h limit.h
	gcc -Wall -ggdb3 marketSim.c market.c stop.c limit.c -lpthread -o marketSim

profile: marketSim.c market.c stop.c limit.c marketSim.h market.h stop.h limit.h
	gcc -Wall -pg marketSim.c market.c stop.c limit.c -lpthread -o marketSim

clean:
	rm -rf marketSim marketSim.exe
