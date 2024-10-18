add_llvm_library(printNamesPluginShmelev MODULE printNamesPluginShmelev.cpp PLUGIN_TOOL clang)

if(WIN32 OR CYGWIN)
  set(LLVM_LINK_COMPONENTS
    Support
  )
  clang_target_link_libraries(printNamesPluginShmelev PRIVATE
    clangAST
    clangBasic
    clangFrontend
    )
endif()

set(CLANG_TEST_DEPS "printNamesPluginShmelev" ${CLANG_TEST_DEPS} PARENT_SCOPE)