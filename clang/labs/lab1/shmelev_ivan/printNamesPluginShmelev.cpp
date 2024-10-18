#include "clang/AST/ASTConsumer.h"
#include "clang/AST/RecursiveASTVisitor.h"
#include "clang/Frontend/CompilerInstance.h"
#include "clang/Frontend/FrontendPluginRegistry.h"
#include "llvm/Support/raw_ostream.h"

class ShmelevPluginPrintNames
    : public clang::RecursiveASTVisitor<ShmelevPluginPrintNames> {
public:
  explicit ShmelevPluginPrintNames(clang::ASTContext *con) : con(con) {}
  bool VisitCXXRecordDecl(clang::CXXRecordDecl *object_decl) {
    llvm::outs() << object_decl->getNameAsString() << "\n";
    for (const auto &declaration : object_decl->decls()) {
      if (clang::FieldDecl *field_decl =
              clang::dyn_cast<clang::FieldDecl>(declaration)) {
        llvm::outs() << "  |_" << field_decl->getNameAsString() << "\n";
      } else if (clang::VarDecl *var_decl =
                     clang::dyn_cast<clang::VarDecl>(declaration)) {
        if (var_decl->isStaticDataMember()) {
          llvm::outs() << "  |_" << var_decl->getNameAsString() << "\n";
        }
      }
    }
    return true;
  }

private:
  clang::ASTContext *con;
};

class Consumer : public clang::ASTConsumer {
public:
  explicit Consumer(clang::ASTContext *con) : guest(con) {}

  virtual void HandleTranslationUnit(clang::ASTContext &con) override {
    guest.TraverseDecl(con.getTranslationUnitDecl());
  }

private:
  ShmelevPluginPrintNames guest;
};

class Plugin : public clang::PluginASTAction {
public:
  virtual std::unique_ptr<clang::ASTConsumer>
  CreateASTConsumer(clang::CompilerInstance &comp_inst,
                    llvm::StringRef in_file) override {
    return std::make_unique<Consumer>(&comp_inst.getASTContext());
  }

  bool ParseArgs(const clang::CompilerInstance &comp_inst,
                 const std::vector<std::string> &arguments) override {
    if (arguments.size() && arguments[0] == "help")
      help(llvm::errs());
    return true;
  }
  void help(llvm::raw_ostream &raw_ost) {
    raw_ost << "This plugin outputs the names of all classes/structures and "
               "their fields.\n";
  }
};

static clang::FrontendPluginRegistry::Add<Plugin>
    X("print-names-plugin-shmelev",
      "Printing the names of all classes/structures and their fields.");