.PHONY: clean dist

dist: clean
	tar -hzcf "$(CURDIR).tar.gz" hashtable/* test/* holdall/* word/* opt/* avl/* rapport/* makefile

clean:
	$(MAKE) -C test clean
