add_llvm_library(PivovarovAlwaysInlinePlugin MODULE PivovarovAlwaysInline.cpp PLUGIN_TOOL clang)

if(WIN32 OR CYGWIN)
  set(LLVM_LINK_COMPONENTS
    Support
  )
  clang_target_link_libraries(PivovarovAlwaysInlinePlugin PRIVATE
    clangAST
    clangBasic
    clangFrontend
    )
endif()

set(CLANG_TEST_DEPS "PivovarovAlwaysInlinePlugin" ${CLANG_TEST_DEPS} PARENT_SCOPE)
