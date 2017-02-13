DFS = dummy_filesys.c

all: $(DFS)
	gcc -w -o dummyFS $(DFS)

run:
	gcc -w -o dummyFS $(DFS)
	./dummyFS

clean:
	rm -rf dummyFS
