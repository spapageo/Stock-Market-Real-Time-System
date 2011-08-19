#CC=gcc


all: marketSim


strip: marketSim
	strip -s marketSim

marketSim: marketSim.c market.c stop.c limit.c marketSim.h market.h stop.h limit.h
<<<<<<< .merge_file_xez3ci
	gcc -Wall -Wextra -O3 -m64 marketSim.c market.c stop.c stoplimit.c limit.c cancel.c -lpthread -o marketSim

clang: marketSim.c market.c stop.c limit.c marketSim.h market.h stop.h limit.h
	clang -Wall -Wextra -O4 marketSim.c market.c stop.c stoplimit.c limit.c cancel.c -lpthread -o marketSim
>>>>>>> .merge_file_nICViq

debug: marketSim.c market.c stop.c limit.c marketSim.h market.h stop.h limit.h
	gcc -Wall -Wextra -g3 marketSim.c market.c stop.c stoplimit.c limit.c cancel.c -lpthread -o marketSim

profile: marketSim.c market.c stop.c limit.c marketSim.h market.h stop.h limit.h
	gcc -Wall -pg -g3 -O3 marketSim.c market.c stop.c stoplimit.c limit.c cancel.c -lpthread -o marketSim

clean:
	rm -rf marketSim marketSim.exe logfile.txt
