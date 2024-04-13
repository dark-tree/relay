
COMMON_DEPS := build/common/logger.o build/common/map.o build/common/set.o build/common/vec.o build/common/stream.o build/common/input.o build/common/util.o
SERVER_DEPS := build/server/group.o build/server/mutex.o build/server/sequence.o build/server/server.o build/server/user.o build/server/tcps.o build/server/store.o
CLIENT_DEPS := build/client/client.o

.PHONY: all
all: build/bin/server build/bin/client

.PHONY: clean
clean:
	@rm -rf build

build/bin/client: $(COMMON_DEPS) $(CLIENT_DEPS)
	@mkdir -p build/bin
	@echo "Linking $@"
	@gcc -g $(COMMON_DEPS) $(CLIENT_DEPS) -o $@

build/bin/server: $(COMMON_DEPS) $(SERVER_DEPS)
	@mkdir -p build/bin
	@echo "Linking $@"
	@gcc -g $(COMMON_DEPS) $(SERVER_DEPS) -o $@

build/common/logger.o: src/common/logger.c
build/common/map.o: src/common/map.c
build/common/util.o: src/common/util.c
build/common/set.o: src/common/set.c
build/common/vec.o: src/common/vec.c
build/common/stream.o: src/common/stream.c
build/common/input.o: src/common/input.c
build/server/group.o: src/server/group.c
build/server/mutex.o: src/server/mutex.c
build/server/sequence.o: src/server/sequence.c
build/server/tcps.o: src/server/tcps.c
build/server/server.o: src/server/server.c
build/server/user.o: src/server/user.c
build/server/store.o: src/server/store.c
build/client/client.o: src/client/client.c

$(COMMON_DEPS) $(SERVER_DEPS) $(CLIENT_DEPS):
	@mkdir -p build/server
	@mkdir -p build/common
	@mkdir -p build/client
	@mkdir -p build
	@echo "Compiling $<"
	@gcc -c -g -Isrc $< -o $@
