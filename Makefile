# pick up examples in src subdir
EXAMPLES=$(sort $(wildcard src/*.c))
# filename sans subdir name and .c extension becomes a target
TARGETS=$(EXAMPLES:src/%.c=%)

RM=/bin/rm

# build/run an example
$(TARGETS): %: src/%.c
	@echo
	@echo "============== Example $@ ======================="
	@echo
	$(CC) -Iinc -undefined -shared -fPIC -o bld/$@.so $<
	@echo "Calling src/t_$@.lua"
	valgrind --leak-check=yes lua src/t_$@.lua

tests: $(TARGETS)
	@$(foreach target, $(TARGETS), make $(target);)

# alternative build sequence (ex01.c example):
#    $(CC) -Iinc -fPIC -shared -c src/ex01.c -o bld/ex01.o
#    $(CC) -fPIC -shared -undefined bld/ex01.o -o ex01.so
#    ./t_ex01.lua

clean:
	$(RM) -f bld/*.so bld/*.o
