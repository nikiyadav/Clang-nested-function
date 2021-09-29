set(LLVM_LINK_COMPONENTS support)

add_clang_executable(clang-nested-func
	clangNestedFunc.cpp
)
target_link_libraries(clang-nested-func
	PRIVATE
	clangTooling
	clangBasic
	clangASTMatchers
)
