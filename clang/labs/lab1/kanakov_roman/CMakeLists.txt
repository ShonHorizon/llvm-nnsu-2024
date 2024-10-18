add_llvm_library(KanakovRenameInd MODULE RenameInd.cpp PLUGIN_TOOL clang)

if(WIN32 OR CYGWIN)
  set(LLVM_LINK_COMPONENTS
    Support
  )
  clang_target_link_libraries(KanakovRenameInd PRIVATE
    clangAST
    clangBasic
    clangFrontend
    )
endif()

set(CLANG_TEST_DEPS "KanakovRenameInd" ${CLANG_TEST_DEPS} PARENT_SCOPE)