# 目标
TARGET := aru
# 单元测试目标
UNIT_TEST_TARGET := $(TARGET)_ut
export ROOT_DIR := $(shell pwd)

# 源码路径
SRC_DIR := src
UT_DIR := unittest

# 临时目录
BUILD_DIR := build
DIST_DIR := dist

# 配置头文件
CONFIG_HEADER := $(BUILD_DIR)/configure.h

# 配置
include config.mk

# 源文件
SOURCES := $(shell find $(SRC_DIR) -type f -name *.cpp)
# 单元测试文件
UT_SOURCES := $(shell find $(UT_DIR) -type f -name *.cpp)

# 替换后缀
OBJECTS := $(patsubst $(SRC_DIR)/%, $(BUILD_DIR)/$(SRC_DIR)/%,$(SOURCES:.cpp=.o))
UT_OBJECTS := $(patsubst $(UT_DIR)/%, $(BUILD_DIR)/$(UT_DIR)/%,$(UT_SOURCES:.cpp=.o))

# 临时依赖文件，用于分析每个.o文件依赖的头文件，在依赖的头文件变化时重新编译.o
DEPS := $(OBJECTS:%.o=%.d)
DEPS += $(UT_OBJECTS:%.o=%.d)

default: $(TARGET)

.PHONY: all
all: pack demo ut

# 生成配置头文件
.PHONY: config
config:
	@mkdir -p build
	@mkdir -p include
	@mkdir -p lib
	@echo "\033[32mpython3 config.py\033[0m"
	@python3 config.py --spath=$(ROOT_DIR)/src --dpath=$(ROOT_DIR)/build \
	--version=$(RELEASE_VERSION) --build-version=$(BUILD_VERSION) --build-time="$(COMPILE_TIME)" \
	--compiler=$(CC) --debug=$(DEBUG)
	@echo "\033[35m[---- generic configure.h success ------]\033[0m"
	@echo ""

$(CONFIG_HEADER):config

$(TARGET): $(OBJECTS)
	@$(CXX) $(SHAREDFLG) $(OBJECTS) -o build/lib$@.so $(SO_LIBS) $(LIBS_PATH)
	@$(AR) build/lib$@.a $(OBJECTS)
	@rm -f $(CONFIG_HEADER)
	@echo "\033[35m[---------- build lib success ----------]\033[0m"
	@echo ""

# 示例代码编译
.PHONY: demo
demo: $(TARGET)
	@make -C samples ARU_INC_DIR=$(ROOT_DIR)/src ARU_LIB_DIR=$(ROOT_DIR)/build OUTPUT_DIR=$(ROOT_DIR)/build/samples

# 单元测试
ut: $(TARGET) $(UT_OBJECTS)
	@$(CXX) $(LIBS_PATH) $(UT_OBJECTS) -l$(TARGET) -Lbuild $(ST_LIBS_UT) $(ST_LIBS) $(SO_LIBS) -o build/$(UNIT_TEST_TARGET)
	@echo "\033[35m[---------- build ut success -----------]\033[0m"
	@echo ""

# Doxygen文档
.PHONY: api
api:

# 打包
.PHONY: pack
pack: $(TARGET) api
	@echo "\033[35m[----------- package start -------------]\033[0m"
	@rm -rf $(DIST_DIR)
	@mkdir -p $(DIST_DIR)/include/aru
	@mkdir -p $(DIST_DIR)/lib
	@mkdir -p $(DIST_DIR)/docs
	@mkdir -p $(DIST_DIR)/demo

	@echo "\033[32mcopy samples\033[0m"
	@cp -f samples/*.cpp $(DIST_DIR)/demo/
	@cp -f samples/Makefile $(DIST_DIR)/demo/

	@echo "\033[32mcopy docs\033[0m"
	@cp -rf docs/usr/* $(DIST_DIR)/docs/
	@cp -f README.md $(DIST_DIR)/

	@echo "\033[32mcopy libs\033[0m"
	@cp -f build/lib$(TARGET).so $(DIST_DIR)/lib/

	@echo "\033[32mcopy headers\033[0m"
	@cp src/aru.hpp $(DIST_DIR)/include/aru/
	@mkdir -p $(DIST_DIR)/include/aru/log/
	@cp src/log/log.hpp $(DIST_DIR)/include/aru/log/
	@mkdir -p $(DIST_DIR)/include/aru/sdk/crypto/
	@cp src/sdk/crypto/md5.hpp $(DIST_DIR)/include/aru/sdk/crypto/

	@echo "\033[32mtar -zcf aru-$(PLAT_NAME)-$(RELEASE_VERSION)-$(MODE).tar.gz include lib demo docs README.md\033[0m"
	@cd $(DIST_DIR) && tar -zcf aru-$(PLAT_NAME)-$(RELEASE_VERSION)-$(MODE).tar.gz include lib demo docs README.md
	@mkdir -p target/$(PLAT_NAME)/$(MODE)/

	@echo "\033[32mcopy aru-$(PLAT_NAME)-$(RELEASE_VERSION)-$(MODE).tar.gz to target/$(PLAT_NAME)/$(MODE)/\033[0m"
	@cp $(DIST_DIR)/aru-$(PLAT_NAME)-$(RELEASE_VERSION)-$(MODE).tar.gz target/$(PLAT_NAME)/$(MODE)/

	@echo "\033[35m[---------- package success ------------]\033[0m"
	@echo ""

.PHONY: clean
clean:
	@echo clean
	@rm -rf $(BUILD_DIR)
	@rm -rf $(DIST_DIR)

# dependencies
-include $(DEPS)
$(BUILD_DIR)/%.o: %.c* $(CONFIG_HEADER)
	@echo "\033[32m$(CXX) $<\033[0m"
	@if [ ! -d $(dir $@) ]; then mkdir -p $(dir $@); fi;\
	$(CXX) $(OBJCCFLAG) -MM -MT $@ -MF $(patsubst %.o, %.d, $@) $<; \
	$(CXX) $(OBJCCFLAG) $< -o $@
