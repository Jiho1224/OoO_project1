cse561sim: cse561sim.c run.c basic.c model.c
	gcc -g -O2 $^ -o $@

.PHONY: clean
clean:
	rm -rf *~ cse561sim

help:
	@echo "The following options are provided with Make\n\t-make:\t\tbuild simulator\n\t-make clean:\tclean the build\n\t-make test:\ttest your simulator"

test: cse561sim test_1 test_2 test_3 test_4 

test_1:
	@echo "Testing sample_input_gcc"; \
	timeout 2 ./cse561sim 60 15 3 ./traces/sample_input_gcc | diff -Naur ./validation/sample_output_gcc -;
	if [ $$? -eq 0 ]; then echo "\tTest seems correct\n"; else echo "\tResults not identical, check the diff output\n"; fi

test_2:
	@echo "Testing sample_input_perl"; \
	timeout 2 ./cse561sim 60 15 3 ./traces/sample_input_perl | diff -Naur ./validation/sample_output_perl -;
	if [ $$? -eq 0 ]; then echo "\tTest seems correct\n"; else echo "\tResults not identical, check the diff output\n"; fi

test_3:
	@echo "Testing val_trace_gcc1"; \
	timeout 2 ./cse561sim 60 15 3 scope ./traces/sample_input_gcc | diff -Naur ./validation/sample_scope_gcc -;
	if [ $$? -eq 0 ]; then echo "\tTest seems correct\n"; else echo "\tResults not identical, check the diff output\n"; fi

test_4:
	@echo "Testing val_trace_perl1"; \
	timeout 2 ./cse561sim 60 15 3 scope ./traces/sample_input_perl | diff -Naur ./validation/sample_scope_perl -;
	if [ $$? -eq 0 ]; then echo "\tTest seems correct\n"; else echo "\tResults not identical, check the diff output\n"; fi
