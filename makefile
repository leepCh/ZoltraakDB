CXX = g++
CXXFLAGS = -std=c++20
TARGET = zoltraak


SRCS = tcpSocket.cpp RESPParser/RespParser.cpp core/evalreq.cpp core/functions.cpp


all: $(TARGET)

$(TARGET): $(SRCS)
	$(CXX) $(CXXFLAGS) $(SRCS) -o $(TARGET)


clean:
	rm -f $(TARGET)
