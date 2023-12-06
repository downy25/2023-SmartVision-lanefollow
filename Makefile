CC = g++
CCFLAGS = -g -Wall

DXLFLAGS = -I/usr/local/include/dynamixel_sdk
DXLFLAGS += -ldxl_x64_cpp
DXLFLAGS += -lrt
OPENCV = `pkg-config opencv4 --cflags --libs`    # opencv 헤더파일경로, 링크할 라이브러리 파일을 자동으로 찾아줌, ` 는 탭키위에 있는 grave accent 문자임


TARGET = lanefollow
OBJS = lanefollow.o dxl.o vision.o
$(TARGET) :  $(OBJS)
	$(CC) $(CCFLAGS) -o $(TARGET) $(OBJS) $(DXLFLAGS) $(OPENCV)
lanefollow.o : lanefollow.cpp dxl.hpp
	$(CC) $(CCFLAGS) -c lanefollow.cpp $(DXLFLAGS) $(OPENCV)
dxl.o : dxl.hpp dxl.cpp
	$(CC) $(CCFLAGS) -c dxl.cpp $(DXLFLAGS) 
vision.o : vision.cpp vision.hpp
	$(CC) $(CCFLAGS) -c vision.cpp $(DXLFLAGS) $(OPENCV)

.PHONY: all clean

all: $(TARGET)

clean:
	rm -rf $(TARGET) $(OBJS)