gcc src/processAserver.c -lbmp -lm -lncurses -o bin/serverA -lrt -pthread
gcc src/processAclient.c -lbmp -lm -lncurses -o bin/clientA -lrt -pthread
gcc src/processA.c -lbmp -lm -lncurses -o bin/processA -lrt -pthread
gcc src/processB.c -lbmp -lm -lncurses -o bin/processB -lrt -pthread
gcc src/master_sc.c -o bin/master_sc -lrt -pthread
