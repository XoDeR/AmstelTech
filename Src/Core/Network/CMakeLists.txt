# Core/Network
# AMSTEL_SOURCES_CORE_NETWORK
set(AMSTEL_SOURCES_CORE_NETWORK_HPP
${CMAKE_CURRENT_SOURCE_DIR}/IpAddress.h
${CMAKE_CURRENT_SOURCE_DIR}/Socket.h
${CMAKE_CURRENT_SOURCE_DIR}/Types.h
)

set(AMSTEL_SOURCES_CORE_NETWORK_HPP
	${AMSTEL_SOURCES_CORE_NETWORK_HPP}
	
	PARENT_SCOPE
)

set(AMSTEL_SOURCES_CORE_NETWORK_CPP
${CMAKE_CURRENT_SOURCE_DIR}/Socket.cpp
)

set(AMSTEL_SOURCES_CORE_NETWORK_CPP
	${AMSTEL_SOURCES_CORE_NETWORK_CPP}
	
	PARENT_SCOPE
)

set(AMSTEL_SOURCES_CORE_NETWORK
	${AMSTEL_SOURCES_CORE_NETWORK_HPP}
	${AMSTEL_SOURCES_CORE_NETWORK_CPP}
	
	${CMAKE_CURRENT_SOURCE_DIR}/CMakeLists.txt
)

set(AMSTEL_SOURCES_CORE_NETWORK
	${AMSTEL_SOURCES_CORE_NETWORK}
	
	PARENT_SCOPE
)