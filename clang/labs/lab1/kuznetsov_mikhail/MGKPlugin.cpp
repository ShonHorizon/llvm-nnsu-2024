#include "clang/AST/ASTConsumer.h"
#include "clang/AST/RecursiveASTVisitor.h"
#include "clang/Frontend/CompilerInstance.h"
#include "clang/Frontend/FrontendPluginRegistry.h"
#include "llvm/Support/raw_ostream.h"

class PrintClassFieldsVisitor
    : public clang::RecursiveASTVisitor<PrintClassFieldsVisitor> {
public:
  bool PrintFields;

  PrintClassFieldsVisitor(bool PrintFields) : PrintFields(PrintFields) {}

  bool VisitCXXRecordDecl(clang::CXXRecordDecl *Declaration) {
    if (Declaration->isClass() || Declaration->isStruct()) {
      llvm::outs() << "Class Name: " << Declaration->getNameAsString() << "\n";
      if (PrintFields) {
        for (auto It = Declaration->decls_begin();
             It != Declaration->decls_end(); ++It) {
          if (auto field = llvm::dyn_cast<clang::FieldDecl>(*It)) {
            llvm::outs() << "|_" << field->getNameAsString() << "\n";
          } else if (auto var = llvm::dyn_cast<clang::VarDecl>(*It)) {
            if (var->isStaticDataMember()) {
              llvm::outs() << "|_" << var->getNameAsString() << "\n";
            }
          }
        }
      }
      llvm::outs() << "\n";
    }
    return true;
  }
};

class PrintClassFieldsConsumer : public clang::ASTConsumer {
public:
  PrintClassFieldsVisitor Visitor;

  PrintClassFieldsConsumer(bool PrintFields) : Visitor(PrintFields) {}

  void HandleTranslationUnit(clang::ASTContext &Context) override {
    Visitor.TraverseDecl(Context.getTranslationUnitDecl());
  }
};

class PrintClassFieldsAction : public clang::PluginASTAction {
public:
  bool PrintFields = true;

  std::unique_ptr<clang::ASTConsumer>
  CreateASTConsumer(clang::CompilerInstance &CI, llvm::StringRef) override {
    return std::make_unique<PrintClassFieldsConsumer>(PrintFields);
  }

  bool ParseArgs(const clang::CompilerInstance &CI,
                 const std::vector<std::string> &Args) override {
    for (const auto &arg : Args) {
      if (arg == "no_fields") {
        PrintFields = false;
      }
    }
    return true;
  }
};

static clang::FrontendPluginRegistry::Add<PrintClassFieldsAction>
    X("prin-elds", "Prints names of all classes and their fields");