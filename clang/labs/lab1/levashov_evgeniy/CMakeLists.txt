add_llvm_library(PluginAlwaysInlineLevashov MODULE AddAttrAlwaysInline.cpp PLUGIN_TOOL clang)

if(WIN32 OR CYGWIN)
  set(LLVM_LINK_COMPONENTS
    Support
  )
  clang_target_link_libraries(PluginAlwaysInlineLevashov PRIVATE
    clangAST
    clangBasic
    clangFrontend
    )
endif()

set(CLANG_TEST_DEPS "PluginAlwaysInlineLevashov" ${CLANG_TEST_DEPS} PARENT_SCOPE)
