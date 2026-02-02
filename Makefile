# 可执行文件名
TARGET    	:= voice_cap

# 源文件：自动扫描当前目录所有 .c
SRCS      	:= $(wildcard source/*.c)

# 两个目录
INCLUDES 	:= -I./include/ -I./lib/inc/

# 中间目标文件
OBJS      	:= $(SRCS:.c=.o)

# 编译器与参数
CFLAGS    	:= $(INCLUDES) -Wall -Wextra -O2 -g -std=c11
LDFLAGS   	:= -L./lib/
LIBS		:= -lwebrtcvad -lasound

CFLAGS 		+= -DDEBUG_SD
CFLAGS		+= -DDEBUG_AC

# 默认规则：生成可执行文件
$(TARGET): $(OBJS)
	$(CC) $^ $(LDFLAGS) $(LIBS) -o $@

# 模式规则：.c → .o
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# 伪目标：清理
clean:
	rm -f $(OBJS) $(TARGET)

# 伪目标：完全重新构建
rebuild: clean $(TARGET)

# 安装（可选）
PREFIX ?= /usr/local
install: $(TARGET)
	install -D $(TARGET) $(DESTDIR)$(PREFIX)/bin/$(TARGET)

# 卸载
uninstall:
	rm -f $(DESTDIR)$(PREFIX)/bin/$(TARGET)

.PHONY: clean rebuild install uninstall
