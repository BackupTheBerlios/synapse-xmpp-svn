
#if (WIN32)
include_directories(
    tools/zip 
    tools/zip/minizip
    tools/zip/minizip/win32
)
#endif (WIN32)

set(zip_noMOC_SRCS tools/zip/zip.cpp tools/zip/minizip/unzip.c)

#set(zip_H tools/zip/zip.h)

if (WIN32)
target_link_libraries(tools/zip/minizip/win32/libz.a)
endif (WIN32)
