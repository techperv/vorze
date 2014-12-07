OBJS:=main.o mplayerudp.o csv.o vorze.o js.o
TARGET:=vorzemplayer

$(TARGET): $(OBJS)
	$(CC) -o $(TARGET) $(OBJS)

clean:
	rm -f $(OBJS)
