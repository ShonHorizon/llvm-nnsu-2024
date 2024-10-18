add_llvm_library(LoginovRenPlugin MODULE LoginovRen.cpp PLUGIN_TOOL clang)

if(WIN32 OR CYGWIN)
  set(LLVM_LINK_COMPONENTS
    Support
  )
  clang_target_link_libraries(LoginovRenPlugin PRIVATE
    clangAST
    clangBasic
    clangFrontend
    )
endif()

set(CLANG_TEST_DEPS "LoginovRenPlugin" ${CLANG_TEST_DEPS} PARENT_SCOPE)
