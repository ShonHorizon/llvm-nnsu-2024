#include "clang/AST/ASTConsumer.h"
#include "clang/AST/RecursiveASTVisitor.h"
#include "clang/Frontend/CompilerInstance.h"
#include "clang/Frontend/FrontendPluginRegistry.h"
#include "clang/Rewrite/Core/Rewriter.h"

class RenameVisitor : public clang::RecursiveASTVisitor<RenameVisitor> {
private:
  clang::Rewriter rewriter_;
  std::string nameOld_;
  std::string nameNew_;

public:
  explicit RenameVisitor(clang::Rewriter rewriter, std::string nameOld,
                         std::string nameNew)
      : rewriter_(rewriter), nameOld_(nameOld), nameNew_(nameNew) {}

  bool VisitFunctionDecl(clang::FunctionDecl *funcDecl) {
    if (funcDecl->getNameAsString() == nameOld_)
      rewriter_.ReplaceText(funcDecl->getNameInfo().getSourceRange(), nameNew_);
    return true;
  }

  bool VisitVarDecl(clang::VarDecl *varDecl) {
    if (varDecl->getNameAsString() == nameOld_)
      rewriter_.ReplaceText(varDecl->getLocation(),
                            varDecl->getNameAsString().size(), nameNew_);
    if (varDecl->getType().getAsString() == nameOld_ ||
        varDecl->getType().getAsString() == nameOld_ + " *")
      rewriter_.ReplaceText(
          varDecl->getTypeSourceInfo()->getTypeLoc().getBeginLoc(),
          nameOld_.size(), nameNew_);
    return true;
  }

  bool VisitCXXRecordDecl(clang::CXXRecordDecl *recordDecl) {
    if (nameOld_ == recordDecl->getName()) {
      rewriter_.ReplaceText(recordDecl->getLocation(), nameNew_);
      const auto destructorDecl = recordDecl->getDestructor();
      if (recordDecl->hasUserDeclaredDestructor())
        rewriter_.ReplaceText(destructorDecl->getLocation(),
                              nameOld_.size() + 1, "~" + nameNew_);
    }
    return true;
  }

  bool VisitDeclRefExpr(clang::DeclRefExpr *refDecl) {
    if (nameOld_ == refDecl->getNameInfo().getAsString())
      rewriter_.ReplaceText(refDecl->getNameInfo().getSourceRange(), nameNew_);
    return true;
  }

  bool VisitCXXNewExpr(clang::CXXNewExpr *newExpr) {
    if (nameOld_ == newExpr->getConstructExpr()->getType().getAsString())
      rewriter_.ReplaceText(newExpr->getExprLoc(), nameOld_.size() + 4,
                            "new " + nameNew_);
    return true;
  }

  bool save() { return rewriter_.overwriteChangedFiles(); }
};

class RenameConsumer : public clang::ASTConsumer {
  RenameVisitor Visitor_;

public:
  explicit RenameConsumer(clang::CompilerInstance &CI, std::string nameOld,
                          std::string nameNew)
      : Visitor_(clang::Rewriter(CI.getSourceManager(), CI.getLangOpts()),
                 nameOld, nameNew) {}
  void HandleTranslationUnit(clang::ASTContext &context) override {
    Visitor_.TraverseDecl(context.getTranslationUnitDecl());
    Visitor_.save();
  }
};

class RenamePlugin : public clang::PluginASTAction {
  std::string nameOld_;
  std::string nameNew_;

protected:
  bool ParseArgs(const clang::CompilerInstance &CI,
                 const std::vector<std::string> &args) override {
    if (args.size() < 2) {
      llvm::errs() << "Params can't be less then two\n";
      return false;
    }

    for (const auto &arg : args) {
      if (arg.find("nameOld_=") == 0)
        nameOld_ = arg.substr(strlen("nameOld_="));
      else if (arg.find("nameNew_=") == 0)
        nameNew_ = arg.substr(strlen("nameNew_="));
    }

    if (nameOld_.empty() || nameNew_.empty()) {
      llvm::errs() << "Params 'nameOld_' and/or 'nameNew_' not found\n";
      return false;
    }
    return true;
  }

public:
  std::unique_ptr<clang::ASTConsumer>
  CreateASTConsumer(clang::CompilerInstance &CI,
                    clang::StringRef InFile) override {
    return std::make_unique<RenameConsumer>(CI, nameOld_, nameNew_);
  }
};

static clang::FrontendPluginRegistry::Add<RenamePlugin>
    X("kanakov-rename-plugin", "Rename variable, function or class");
