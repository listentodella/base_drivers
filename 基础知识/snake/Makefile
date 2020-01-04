APPS = snake

all:$(APPS)

snake:snake.o
	gcc -o $@ $^ -Wall

%.o:%.c
	gcc -c -o $@ $<

clean:
	rm $(APPS) -f; rm -f *.o
