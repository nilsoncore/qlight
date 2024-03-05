SOURCE_DIRECTORY := src
BUILD_DIRECTORY  := build
OBJECT_DIRECTORY := $(BUILD_DIRECTORY)/obj

OUTPUT_EXECUTABLE := $(BUILD_DIRECTORY)/Debug_x64/qlight
SOURCE_FILES      := $(wildcard $(SOURCE_DIRECTORY)/*.cpp)
OBJECT_FILES      := $(SOURCE_FILES:$(SOURCE_DIRECTORY)/%.cpp=$(OBJECT_DIRECTORY)/%.o)

INCLUDE_PATHS_BY_SPACE := libs $(VULKAN_SDK)/include
LIBRARY_PATHS_BY_SPACE := libs/GLFW libs/GLEW $(VULKAN_SDK)/lib

INCLUDE_PATHS_ARGUMENT := $(foreach include_path, $(INCLUDE_PATHS_BY_SPACE), -I $(include_path))
LIBRARY_PATHS_ARGUMENT := $(foreach library_path, $(LIBRARY_PATHS_BY_SPACE), -L $(library_path))

CXX      := clang++
CXXFLAGS := -Wall
CPPFLAGS := $(INCLUDE_PATHS_ARGUMENT) -MMD -MP
LDFLAGS  := $(LIBRARY_PATHS_ARGUMENT)
LDLIBS   :=

.PHONY: all
all: $(OUTPUT_EXECUTABLE)

$(OUTPUT_EXECUTABLE): $(OBJECT_FILES) $(BUILD_DIRECTORY) $(OBJECT_DIRECTORY)
	@echo "Source directory: $(SOURCE_DIRECTORY)"
	@echo "Build directory: $(BUILD_DIRECTORY)"
	@echo "Object directory: $(OBJECT_DIRECTORY)"
	@echo "Output executable: $(OUTPUT_EXECUTABLE)"
	@echo "Source files: $(SOURCE_FILES)"
	@echo "Object files: $(OBJECT_FILES)"
	@echo "Include paths: $(INCLUDE_PATHS_BY_SPACE)"
	@echo "Library paths: $(LIBRARY_PATHS_BY_SPACE)"
	@echo "Include paths argument: $(INCLUDE_PATHS_ARGUMENT)"
	@echo "Library paths argument: $(LIBRARY_PATHS_ARGUNENT)"
	$(CXX) $(LDFLAGS) $^ $(LDLIBS) -o $@

$(OBJECT_DIRECTORY) $(BUILD_DIRECTORY):
	mkdir -p $@

$(OBJECT_DIRECTORY)/%.o: $(SOURCE_DIRECTORY)/%.cpp $(OBJECT_DIRECTORY)
	$(CXX) $(CPPFLAGS) $(CFLAGS) -c $< -o $@

.PHONE: clean
clean:
	rm -f $(OUTPUT_DIRECTORY)/*.o
	rm -f $(OUTPUT_EXECUTABLE)

-include $(OBJECT_FILES:.o=.d)