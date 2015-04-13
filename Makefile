.PHONY: all

all:
	cd src; make

clean:
	rm exec/as3
	rm exec/*.o