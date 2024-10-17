add_llvm_library(MGKPlugin MODULE MGKPlugin.cpp PLUGIN_TOOL clang)

if(WIN32 OR CYGWIN)
  set(LLVM_LINK_COMPONENTS
    Support
  )
  clang_target_link_libraries(MGKPlugin PRIVATE
    clangAST
    clangBasic
    clangFrontend
    )
endif()

set(CLANG_TEST_DEPS "MGKPlugin" ${CLANG_TEST_DEPS} PARENT_SCOPE)

