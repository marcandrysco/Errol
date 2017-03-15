all:
	scons optimize=1 prefix=. install
	scons -c

test:
	./run_tests --list | tr -d '<' | xargs ./run_tests

.PHONY: test all
