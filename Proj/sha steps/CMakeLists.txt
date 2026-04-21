set(SHASteps_HEADER_FILES
	include/SHA256.h
	include/SHA256\ Steps.h
	include/OpsLog.h
)

add_executable(SHASteps
		${SHASteps_HEADER_FILES}
		src/SHA256.cpp
		src/SHA256\ Transform.cpp
		src/OrderOfOperations.cpp
		src/OpsLog.cpp

		src/Main.cpp

		Links.txt
)

set_property(TARGET SHASteps PROPERTY FOLDER "SHA Steps")

# Compiler settings
target_include_directories(SHASteps
						   INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/include
						   PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include ${CMAKE_SOURCE_DIR}/include
)

#set_property(TARGET SHASteps PROPERTY VS_DEBUGGER_COMMAND_ARGUMENTS "-pause -run log 1")
set_property(TARGET SHASteps PROPERTY VS_DEBUGGER_COMMAND_ARGUMENTS "-pause -run orderOps ep0")