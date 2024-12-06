CC = g++
CFLAGS = -Wall -g
TARGET = cache-sim

$(TARGET): main.cpp
	$(CC) $(CFLAGS) -o $(TARGET) main.cpp

clean:
	rm -f $(TARGET)
