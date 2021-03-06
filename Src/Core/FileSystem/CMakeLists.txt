# Core/FileSystem
# AMSTEL_SOURCES_CORE_FILE_SYSTEM
set(AMSTEL_SOURCES_CORE_FILE_SYSTEM_HPP
${CMAKE_CURRENT_SOURCE_DIR}/File.h
${CMAKE_CURRENT_SOURCE_DIR}/FileSystem.h
${CMAKE_CURRENT_SOURCE_DIR}/FileSystemApk_Android.h
${CMAKE_CURRENT_SOURCE_DIR}/FileSystemDisk.h
${CMAKE_CURRENT_SOURCE_DIR}/Path.h
${CMAKE_CURRENT_SOURCE_DIR}/ReaderWriter.h
${CMAKE_CURRENT_SOURCE_DIR}/Types.h
)

set(AMSTEL_SOURCES_CORE_FILE_SYSTEM_HPP
	${AMSTEL_SOURCES_CORE_FILE_SYSTEM_HPP}
	
	PARENT_SCOPE
)

set(AMSTEL_SOURCES_CORE_FILE_SYSTEM_CPP
${CMAKE_CURRENT_SOURCE_DIR}/FileSystemApk_Android.cpp
${CMAKE_CURRENT_SOURCE_DIR}/FileSystemDisk.cpp
${CMAKE_CURRENT_SOURCE_DIR}/Path.cpp
)

if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
	set_source_files_properties(${CMAKE_CURRENT_SOURCE_DIR}/FileSystemApk_Android.cpp 
		PROPERTIES HEADER_FILE_ONLY ON)
endif()

# Does this work with MSVC?
# To check
if(MSVC)
	set_source_files_properties(${CMAKE_CURRENT_SOURCE_DIR}/FileSystemApk_Android.cpp 
		PROPERTIES HEADER_FILE_ONLY ON)
	set_source_files_properties(${CMAKE_CURRENT_SOURCE_DIR}/FileMonitor_Linux.cpp 
		PROPERTIES HEADER_FILE_ONLY ON)
endif()

set(AMSTEL_SOURCES_CORE_FILE_SYSTEM_CPP
	${AMSTEL_SOURCES_CORE_FILE_SYSTEM_CPP}
	
	PARENT_SCOPE
)

set(AMSTEL_SOURCES_CORE_FILE_SYSTEM
	${AMSTEL_SOURCES_CORE_FILE_SYSTEM_HPP}
	${AMSTEL_SOURCES_CORE_FILE_SYSTEM_CPP}
	
	${CMAKE_CURRENT_SOURCE_DIR}/CMakeLists.txt
)

set(AMSTEL_SOURCES_CORE_FILE_SYSTEM
	${AMSTEL_SOURCES_CORE_FILE_SYSTEM}
	
	PARENT_SCOPE
)