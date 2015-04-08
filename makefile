OBJECTS = sish.o
sish: $(OBJECTS)
	g++ $^ -o sish
%.o: %.cpp
	g++ -c $< -o $@
clean:
	rm -f *.o sish