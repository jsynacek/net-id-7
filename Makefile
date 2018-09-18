all:
	gcc udev-builtin-net_id.c -o net-id-7 -ludev
clean:
	rm -f *.o net-id-7
