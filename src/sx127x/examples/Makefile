all: example_tx example_rx example_loopback example_cad

example_tx: common.c example_tx.c
	$(CC) -std=c99 -I../ $^ -o $@

example_rx: common.c example_rx.c
	$(CC) -std=c99 -I../ $^ -o $@

example_loopback: common.c example_loopback.c
	$(CC) -std=c99 -I../ $^ -o $@

example_cad: common.c example_cad.c
	$(CC) -std=c99 -I../ $^ -o $@

clean:
	rm -f example_tx example_rx example_loopback example_cad
