project(ast)

set(${PROJECT_NAME}_HDR
	${MINT_INCLUDE_DIR}/${PROJECT_NAME}/abstractsyntaxtree.h
	${MINT_INCLUDE_DIR}/${PROJECT_NAME}/cursor.h
	${MINT_INCLUDE_DIR}/${PROJECT_NAME}/debuginfos.h
	${MINT_INCLUDE_DIR}/${PROJECT_NAME}/module.h
	${MINT_INCLUDE_DIR}/${PROJECT_NAME}/node.h
)

set(${PROJECT_NAME}_SRC
	abstractsyntaxtree.cpp
	cursor.cpp
	debuginfos.cpp
	module.cpp
	node.cpp
	threadentrypoint.cpp
	threadentrypoint.h
)

add_library(${PROJECT_NAME}
	OBJECT
	${${PROJECT_NAME}_SRC}
)

set_target_properties(${PROJECT_NAME} PROPERTIES COMPILE_FLAGS "-fPIC")

# Install
install_headers(${${PROJECT_NAME}_HDR} SUBDIR ${PROJECT_NAME})
