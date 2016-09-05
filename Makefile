mod_name = lkm_queue
obj-m += lkm_queue.o
CFLAGS_lkm_queue.o += -DDEBUG
CXX_FLAGS = -std=c++11 -O3
writer = writer
reader = reader
queue_test = queue_test

all: build

build: mod writer reader

mod:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules

writer:
	$(CXX) -o $(writer) $(CXX_FLAGS) $(writer).cpp

reader:
	$(CXX) -o $(reader) $(CXX_FLAGS) $(reader).cpp

rmmod:
	sudo rmmod $(mod_name) || true

insmod:
	sudo insmod $(mod_name).ko

reload_mod: mod rmmod insmod

# queue test; not build by default
queue_test:
	$(CC) -o $(queue_test) $(queue_test).c

clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean
	rm -fr $(writer) $(reader) $(queue_test)
