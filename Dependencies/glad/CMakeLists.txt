# 3.23+ for target_sources(FILE_SET)
cmake_minimum_required (VERSION 3.23)
project (glad LANGUAGES C)

add_library (glad)

target_sources (glad
	PRIVATE
		src/gl.c
	
	PUBLIC
		FILE_SET gladHeaders
		TYPE HEADERS
		BASE_DIRS include
		FILES
			include/glad/gl.h
			include/KHR/khrplatform.h
)

# install command needs to reference the file set again
install(TARGETS glad FILE_SET gladHeaders)