APPS = list

all:$(APPS)

list:list.o list_operation.o
	gcc -o $@ $^ -Wall

%.o:%.c
	gcc -c -o $@ $<

clean:
	rm $(APPS) -f; rm -f *.o
