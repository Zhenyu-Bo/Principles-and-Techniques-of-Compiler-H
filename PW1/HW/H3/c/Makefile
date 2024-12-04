CC = gcc
CFLAGS = -Wall -g
SRC = sort.c
TARGET = sort
TARGETI = $(TARGET).i# 预处理后的中间文件名
TARGETS = $(TARGET).s# 编译后的中间文件名
TARGETO = $(TARGET).o# 汇编后的中间文件名

# 检查命令行参数，如果是make clang，则使用clang编译器
ifeq ($(MAKECMDGOALS),clang)
	CC = clang
endif

all: preprocess compile assemble link

# 预处理
preprocess: $(SRC)
	$(CC) $(CFLAGS) -E $(SRC) -o $(TARGETI)

# 编译
compile: preprocess
	$(CC) $(CFLAGS) -S $(TARGETI) -o $(TARGETS)

# 汇编
assemble: compile
	$(CC) $(CFLAGS) -c $(TARGETS) -o $(TARGETO)

# 链接
link: assemble
	$(CC) $(CFLAGS) $(TARGETO) -o $(TARGET)

# 生成32位汇编文件
m32: $(SRC)
	$(CC) $(CFLAGS) -S -m32 $(SRC) -o $(TARGET)_32.s

# 生成64位汇编文件
m64: $(SRC)
	$(CC) $(CFLAGS) -S -m64 $(SRC) -o $(TARGET)_64.s

# 默认目标
$(TARGET): link

clang: all

clean:
	rm -f $(TARGET) $(TARGET)_32.s $(TARGET)_64.s $(TARGETI) $(TARGETS) $(TARGETO)
