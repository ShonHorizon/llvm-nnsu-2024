#include "clang/AST/ASTConsumer.h"
#include "clang/AST/RecursiveASTVisitor.h"
#include "clang/Frontend/CompilerInstance.h"
#include "clang/Frontend/FrontendPluginRegistry.h"
#include "clang/Rewrite/Core/Rewriter.h"

#include "llvm/ADT/StringRef.h"
#include "llvm/Support/raw_ostream.h"

class RenameVisitor : public clang::RecursiveASTVisitor<RenameVisitor> {
private:
  clang::Rewriter rewriter;
  std::string oldName;
  std::string newName;

public:
  explicit RenameVisitor(clang::Rewriter rewriter, std::string oldName,
                         std::string newName)
      : rewriter(rewriter), oldName(oldName), newName(newName){};

  bool VisitFunctionDecl(clang::FunctionDecl *FuncDecl) {
    std::string name = FuncDecl->getNameAsString();
    if (name == oldName) {
      rewriter.ReplaceText(FuncDecl->getNameInfo().getSourceRange(), newName);
      rewriter.overwriteChangedFiles();
    }
    return true;
  }

  bool VisitDeclRefExpr(clang::DeclRefExpr *DRE) {
    std::string name = DRE->getNameInfo().getAsString();
    if (name == oldName) {
      rewriter.ReplaceText(DRE->getNameInfo().getSourceRange(), newName);
      rewriter.overwriteChangedFiles();
    }
    return true;
  }

  bool VisitVarDecl(clang::VarDecl *declVar) {
    std::string name = declVar->getNameAsString();
    if (name == oldName) {
      rewriter.ReplaceText(declVar->getLocation(), name.size(), newName);
      rewriter.overwriteChangedFiles();
    }
    if (declVar->getType().getAsString() == oldName + " *" ||
        declVar->getType().getAsString() == oldName) {
      rewriter.ReplaceText(
          declVar->getTypeSourceInfo()->getTypeLoc().getBeginLoc(), name.size(),
          newName);
      rewriter.overwriteChangedFiles();
    }
    return true;
  }

  bool VisitCXXRecordDecl(clang::CXXRecordDecl *CXXRDecl) {
    std::string name = CXXRDecl->getNameAsString();
    if (name == oldName) {
      rewriter.ReplaceText(CXXRDecl->getLocation(), name.size(), newName);
      const clang::CXXDestructorDecl *CXXDD = CXXRDecl->getDestructor();
      if (CXXDD)
        rewriter.ReplaceText(CXXDD->getLocation(), name.size() + 1,
                             "~" + newName);
      rewriter.overwriteChangedFiles();
    }
    return true;
  }

  bool VisitCXXNewExpr(clang::CXXNewExpr *CXXNExpr) {
    std::string name = CXXNExpr->getConstructExpr()->getType().getAsString();
    if (name == oldName) {
      rewriter.ReplaceText(CXXNExpr->getExprLoc(), name.size() + 4,
                           "new " + newName);
      rewriter.overwriteChangedFiles();
    }
    return true;
  }
};

class RenameIdConsumer : public clang::ASTConsumer {
  RenameVisitor visitor;

public:
  explicit RenameIdConsumer(clang::CompilerInstance &compInst,
                            std::string oldName, std::string newName)
      : visitor(clang::Rewriter(compInst.getSourceManager(),
                                compInst.getLangOpts()),
                oldName, newName) {}
  void HandleTranslationUnit(clang::ASTContext &Context) override {
    visitor.TraverseDecl(Context.getTranslationUnitDecl());
  }
};

class RenameIdPlugin : public clang::PluginASTAction {
private:
  std::string oldName;
  std::string newName;

protected:
  bool ParseArgs(const clang::CompilerInstance &Compiler,
                 const std::vector<std::string> &args) override {
    oldName = args[0];
    newName = args[1];
    if (oldName.find("=") == 0 || oldName.find("=") == std::string::npos) {
      llvm::errs() << "Error in the `oldName` parameter.\n";
    }
    if (newName.find("=") == 0 || newName.find("=") == std::string::npos) {
      llvm::errs() << "Error in the `newName` parameter.\n";
    }
    oldName = oldName.substr(oldName.find("=") + 1);
    newName = newName.substr(newName.find("=") + 1);
    return true;
  }

public:
  std::unique_ptr<clang::ASTConsumer>
  CreateASTConsumer(clang::CompilerInstance &Compiler,
                    llvm::StringRef InFile) override {
    return std::make_unique<RenameIdConsumer>(Compiler, oldName, newName);
  }
};

static clang::FrontendPluginRegistry::Add<RenameIdPlugin>
    X("loginov-rename-plugin", "Renaming an identifier");
