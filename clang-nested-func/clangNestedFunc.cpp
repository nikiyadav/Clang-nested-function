/*
***************** references *******************
https://clang.llvm.org/docs/LibASTMatchersReference.html
https://clang.llvm.org/doxygen/namespaceclang_1_1ast__matchers.html
http://clang.llvm.org/docs/RAVFrontendAction.html
**************** View AST **********************
clang -cc1 -ast-dump your_file.c
*/

#include "clang/Frontend/FrontendActions.h"
#include "clang/Tooling/CommonOptionsParser.h"
#include "clang/Tooling/Tooling.h"
#include "llvm/Support/CommandLine.h"
#include "clang/ASTMatchers/ASTMatchers.h"
#include "clang/ASTMatchers/ASTMatchFinder.h"
#include "clang/AST/AST.h"
#include "clang/AST/ASTConsumer.h"
#include "clang/AST/RecursiveASTVisitor.h"
#include "clang/Frontend/CompilerInstance.h"
#include <string>
#include "clang/Rewrite/Core/Rewriter.h"
#include "llvm/Support/raw_ostream.h"
#include <set>
#include <bits/stdc++.h> 

using namespace clang::tooling;
using namespace llvm;
using namespace clang;
using namespace clang::ast_matchers;
using namespace clang::driver;
using namespace std;

CompilerInstance *CI;
std::set <string> rewrittenFunctions;
std::set <string> functionsThatContainLabelStmt;
std::map <string, set<const VarDecl *> > func_varDecl_map;
std::map <string, set<const RecordDecl *> > func_recordDecl_map;
ASTContext *astContext;

// Apply a custom category to all command-line options so that they are the
// only ones displayed.
static llvm::cl::OptionCategory MyToolCategory("my-tool options");

// CommonOptionsParser declares HelpMessage with a description of the common
// command-line options related to the compilation database and input files.
// It's nice to have this help message in all tools.
static cl::extrahelp CommonHelp(CommonOptionsParser::HelpMessage);

// A help message for this specific tool can be added afterwards.
static cl::extrahelp MoreHelp("\nMore help text...\n");

class RewriteLabelStmtHandler : public MatchFinder::MatchCallback {
public:
  RewriteLabelStmtHandler (Rewriter &Rewrite) : Rewrite(Rewrite) { current_func ="";}

  string getFunctionWithDecl(string var, string LSname)
  {
    string name = "";
    if (func_varDecl_map.find(LSname) != func_varDecl_map.end()) 
    {
      for (auto it=func_varDecl_map[LSname].begin(); it!=func_varDecl_map[LSname].end(); it++) 
      {
        name = (*it)->getNameAsString();
        if (name == var) {
          return LSname;
        }
      }
    }
    for (auto it=hierarchy_list.begin(); it!= hierarchy_list.end(); it++) {
      if (func_varDecl_map.find(*it) != func_varDecl_map.end()) {
        for (auto jt = func_varDecl_map[*it].begin(); jt != func_varDecl_map[*it].end(); jt++) 
        {
          name = (*jt)->getNameAsString();
          if (name == var) {
            return *it;
          }
        }
      }
    }

    for (auto jt = func_varDecl_map["global_varDecl"].begin(); jt != func_varDecl_map["global_varDecl"].end(); jt++) 
    {
      name = (*jt)->getNameAsString();
      if (name == var) {
        return "global_varDecl";
      }
    }    

  }

  void InstrumentStmt(const Stmt *ST, string LSname) 
  {
    for (Stmt::const_child_iterator i = ST->child_begin(), e = ST->child_end(); i != e; ++i) {
      const Stmt *currStmt = *i;
      if (isa<clang::MemberExpr> (currStmt)) {
        // currStmt->dump();
        const clang::MemberExpr *memExpr = (const MemberExpr *) currStmt;
        const clang::ValueDecl *valD = memExpr->getMemberDecl();
        // valD->dump();
        string valName = valD->getNameAsString();
        // llvm::outs() << "valName = " << valName << "\n";
        const Expr *base = memExpr->getBase();
        // llvm::outs() <<"dumping base \n";
        // base->dump();
        base = base->IgnoreImpCasts ();
        string baseName;
        if (base && isa<clang::DeclRefExpr>(base))
        {
          const DeclRefExpr *declRef = (const DeclRefExpr *) base;
          // get a var name
          baseName = (declRef->getNameInfo ()).getName ().getAsString ();
        }
        string func = getFunctionWithDecl(baseName, LSname);
        if (func == "") {
          llvm::outs() << "ERROR, did not find proper varDecl\n";
        }

        if (func != "global_varDecl" && func != LSname) {
          Rewrite.ReplaceText(SourceRange(memExpr->getBeginLoc(), memExpr->getEndLoc()),
              "("+ func +"_v."+ baseName + " -> "+ valName +")");
        }
      }
      else if (isa<clang::ArraySubscriptExpr>(currStmt)) 
      {
        const clang::ArraySubscriptExpr *arrExpr = (const ArraySubscriptExpr *) currStmt;
        const Expr *base = arrExpr->getBase();
        base = base->IgnoreImpCasts ();
        const Expr *idx = arrExpr->getIdx();
        idx = idx->IgnoreImpCasts();
  
        std::string name; 
        std::string index;
        if (base && isa<clang::DeclRefExpr>(base))
        {
          const DeclRefExpr *declRef = (const DeclRefExpr *) base;
          // get a var name
          name = (declRef->getNameInfo ()).getName ().getAsString ();
        }
        if (idx && isa<clang::IntegerLiteral> (idx)) {
          const IntegerLiteral *integerLiteral = (const IntegerLiteral *) idx;
          std::string TypeS;
          llvm::raw_string_ostream s (TypeS);
          clang::LangOptions LangOpts;
          LangOpts.CPlusPlus = true;
          clang::PrintingPolicy Policy(LangOpts);
          integerLiteral->printPretty (s, 0, Policy);
          index = s.str();
        }
        else if (idx && isa<clang::DeclRefExpr>(idx) )
        {
          const DeclRefExpr *declRefIdx = (const DeclRefExpr *) idx;
          // get index
          index = (declRefIdx->getNameInfo ()).getName().getAsString ();
        }

        string func = getFunctionWithDecl(name, LSname);
        if (func == "") {
          llvm::outs() << "ERROR, did not find proper varDecl\n";
        }

        if (func != "global_varDecl" && func != LSname) {
          Rewrite.ReplaceText(SourceRange(arrExpr->getBeginLoc(), arrExpr->getEndLoc()),
              "*("+ func +"_v."+ name + " + "+ index +")");
        }
      }
      else if (isa<clang::DeclRefExpr>(currStmt)) {
        const DeclRefExpr *declRef = (DeclRefExpr *) currStmt;
        if (!declRef->getDecl()->isFunctionOrFunctionTemplate()) {
          //currStmt->dump();
          string varName = declRef->getNameInfo().getName().getAsString();
          string func = getFunctionWithDecl(varName, LSname);
          if (func == "") {
            llvm::outs() << "ERROR, did not find proper varDecl\n";
          }

          if (func != "global_varDecl" && func != LSname) {
            Rewrite.ReplaceText(SourceRange(declRef->getBeginLoc(), declRef->getEndLoc()),
                "(*"+ func +"_v."+declRef->getNameInfo().getName().getAsString()+")");
          }
          else 
          {
            Rewrite.ReplaceText(SourceRange(declRef->getBeginLoc(), declRef->getEndLoc()),
                declRef->getNameInfo().getName().getAsString());
          }
        }
      }
      else {
        InstrumentStmt(currStmt, LSname);
      }
    }
  }

  void rewriteLabelStmt(const LabelStmt *LS) {
    bool invalid;
    const char* funcName = LS->getName();
    char funcDeclaration[256];
    sprintf(funcDeclaration, "void %s ()\n", funcName);
    string func_name(funcName);
    rewrittenFunctions.insert(func_name);

    Rewrite.InsertText(Floc, funcDeclaration, true, true);
    Rewrite.InsertText(Floc, "{\n", true, true);
    functionsThatContainLabelStmt.insert(func_name);    

    const Stmt *ST = LS->getSubStmt();
    for (Stmt::const_child_iterator i = ST->child_begin(), e = ST->child_end(); i != e; ++i) {
      const Stmt *currStmt = *i;

      if(!currStmt)
        continue;
      
      /* If currStmt is a labelStmt, we don't write it.*/
      if (isa<clang::LabelStmt> (currStmt))
        continue;

      /* Instrument stmt before rewriting as a function body. */
      InstrumentStmt(currStmt, string(LS->getName()));

      string tmp = Rewrite.getRewrittenText(SourceRange(currStmt->getBeginLoc(),currStmt->getEndLoc().getLocWithOffset(4)));
      int tmp_len = tmp.length();
      char tmp_array[tmp_len+1];
      strcpy(tmp_array, tmp.c_str());
      char tmp_array2[1000];
      sprintf(tmp_array2, "\t%s", tmp_array);
      Rewrite.InsertText(Floc, tmp_array2, true, true);

      /* If it is a declStmt, initialise struct members.*/
      if (isa<clang::DeclStmt>(currStmt))
      {
        const DeclStmt *DS = cast<DeclStmt>(currStmt);
        if(DS->isSingleDecl()) {
          //DS->dump();
          const Decl *d = DS->getSingleDecl();
          int fn = func_name.length();
          char func_name_array[fn+1];
          strcpy(func_name_array, func_name.c_str());
          if (const VarDecl *V = dyn_cast<VarDecl>(d))
          {
            func_varDecl_map[func_name].insert(V); 
            string name = V->getNameAsString();
            int n = name.length();
            char name_array[n+1];
            strcpy(name_array, name.c_str());
            char structInitial[1000];
            if (V->getType().getTypePtr()->isConstantArrayType()) {
              sprintf(structInitial, "\n\t%s_v.%s = %s;", func_name_array, name_array, name_array);
            }
            else if (V->getType().getTypePtr()->isStructureType()) {
              sprintf(structInitial, "\n\t%s_v.%s = &%s;", func_name_array, name_array, name_array);
            }
            else {
              sprintf(structInitial, "\n\t%s_v.%s = &%s;", func_name_array, name_array, name_array);
            }
            Rewrite.InsertText(Floc, structInitial, true, true);
          }
          else if (const RecordDecl *R = dyn_cast<RecordDecl>(d)) 
          {
            // R->dump();
            func_recordDecl_map[func_name].insert(R);
          }
        }
        else {
          for(DeclStmt::const_decl_iterator it = DS->decl_begin(); it != DS->decl_end(); it++ ) {
            const Decl *d = *it;
            const VarDecl *V = cast<VarDecl>(d);
            string name = V->getNameAsString();
            int n = name.length();
            char name_array[n+1];
            strcpy(name_array, name.c_str());
            int fn = func_name.length();
            char func_name_array[fn+1];
            strcpy(func_name_array, func_name.c_str());
            char structInitial[256];
            if (V->getType().getTypePtr()->isConstantArrayType()) {
              sprintf(structInitial, "\n\t%s_v.%s = %s;\n", func_name_array, name_array, name_array);
            }
            else {
              sprintf(structInitial, "\n\t%s_v.%s = &%s;\n", func_name_array, name_array, name_array);
            }
            // sprintf(structInitial, "\t%s_v.%s = &%s;\n", func_name_array, name_array, name_array);
            Rewrite.InsertText(Floc, structInitial, true, true);
          }
        }
      }
    }
    Rewrite.InsertText(Floc, "}\n\n", true, true);
    Rewriter::RewriteOptions opts;
    Rewrite.RemoveText(CharSourceRange(SourceRange(LS->getBeginLoc(), LS->getEndLoc().getLocWithOffset(1)), false), opts);
  }

  void nestedVisit(const LabelStmt *LS) {
    const Stmt *stmt = LS->getSubStmt();
    for (Stmt::const_child_iterator i = stmt->child_begin(), e = stmt->child_end(); i != e; ++i) {
      const Stmt *currStmt = *i;
      if(!currStmt)
        continue;
      
      if (isa<clang::DeclStmt>(currStmt))
      {
        const DeclStmt *DS = cast<DeclStmt>(currStmt);
        if(DS->isSingleDecl()) {
          //DS->dump();
          const Decl *d = DS->getSingleDecl();
           if (const VarDecl *V = dyn_cast<VarDecl>(d))
          {
            // const VarDecl *V = cast<VarDecl>(d);
            /*insert in func_varDecl map*/
            func_varDecl_map[string(LS->getName())].insert(V);
          }
          else if (const RecordDecl *R = dyn_cast<RecordDecl>(d)) 
          {
            // R->dump();
            func_recordDecl_map[string(LS->getName())].insert(R);
          }
        }
        else {
          for(DeclStmt::const_decl_iterator it = DS->decl_begin(); it != DS->decl_end(); it++ ) {
            const Decl *d = *it;
            const VarDecl *V = cast<VarDecl>(d);
            /*insert in func_varDecl map*/
            func_varDecl_map[string(LS->getName())].insert(V);
          }
        }
      }

      if (isa<clang::LabelStmt> (currStmt)) {
        hierarchy_list.push_front(string(LS->getName()));
        const LabelStmt *L = cast<LabelStmt>(currStmt);
        nestedVisit(L);
      }
    }
  
    rewriteLabelStmt(LS);
    hierarchy_list.pop_front();
  }
  
  virtual void run(const MatchFinder::MatchResult &Result) {
    const LabelStmt *LS = Result.Nodes.getNodeAs<clang::LabelStmt>("labelStmt");
    //LS->getSubStmt()->dump();
    DeclContext *DC = LS->getDecl()->getParentFunctionOrMethod();
    FunctionDecl *F = cast<FunctionDecl> (DC);
    string Fname =  F->getNameInfo().getName().getAsString();
    functionsThatContainLabelStmt.insert(Fname);
    Floc = F->getBeginLoc();

    if (hierarchy_list.empty()) {
      current_func = Fname;
      hierarchy_list.push_front(Fname);
    }

    const char* funcName = LS->getName();
    string _funcName(funcName);
    if (rewrittenFunctions.find(_funcName) == rewrittenFunctions.end())
    {
      if (current_func != Fname) {
        if (!hierarchy_list.empty()) {
          hierarchy_list.erase(hierarchy_list.begin(), hierarchy_list.end());
        }
        current_func = Fname;
        hierarchy_list.push_front(Fname);
      }
      nestedVisit(LS);
    }
  }

private:
  Rewriter &Rewrite;
  SourceLocation Floc;
  list<string> hierarchy_list;
  string current_func;
};

class MyASTVisitor: public RecursiveASTVisitor<MyASTVisitor> {
  public:
  MyASTVisitor (Rewriter &Rewrite):  Rewrite(Rewrite) {}

  bool VisitFunctionDecl(FunctionDecl *F) 
  {
    SourceLocation firstStmtLoc;
    int count = 0;
    if (F->hasBody())
    {
      Stmt *stmt = F->getBody();
      string func_name = F->getNameInfo().getName().getAsString();

      /* check that this function has label stmts or not.*/
      bool containLabelStmt = false;
      for (Stmt::child_iterator i = stmt->child_begin(), e = stmt->child_end(); i != e; ++i) {
        Stmt *currStmt = *i;
        if ( count == 0 )
        {
          firstStmtLoc = currStmt->getBeginLoc();
          count++;
        }
        if(!currStmt)
          continue;
        if (isa<clang::LabelStmt> (currStmt)) {
          containLabelStmt = true;  
          break;
        }
      }
      
      if (containLabelStmt) 
      {
        /*Handle function parameters.*/
        ArrayRef< ParmVarDecl * > params = F->parameters();
        for(unsigned int paramIndex=0; paramIndex< params.size(); paramIndex++){
          VarDecl *V = cast<VarDecl> (params[paramIndex]);
          /*insert in func_varDecl map*/
          func_varDecl_map[func_name].insert(V);
          
          string name = V->getNameAsString();
          int n = name.length();
          char name_array[n+1];
          strcpy(name_array, name.c_str());
          int fn = func_name.length();
          char func_name_array[fn+1];
          strcpy(func_name_array, func_name.c_str());
          char structInitial[1000];
          if (V->getType().getTypePtr()->isConstantArrayType()) {
            sprintf(structInitial, "\t%s_v.%s = %s;\n", func_name_array, name_array, name_array);
          }
          else {
            sprintf(structInitial, "\t%s_v.%s = &%s;\n", func_name_array, name_array, name_array);
          }
          Rewrite.InsertText(firstStmtLoc, structInitial, true, true);
        }

        for (Stmt::child_iterator i = stmt->child_begin(), e = stmt->child_end(); i != e; ++i) {
          Stmt *currStmt = *i;
          if(!currStmt)
            continue;
          if (isa<clang::LabelStmt> (currStmt)) {
            continue;  
          }
          if (isa<clang::DeclStmt>(currStmt))
          {
            DeclStmt *DS = cast<DeclStmt>(currStmt);
            SourceLocation s = DS->getEndLoc();
            if(DS->isSingleDecl()) {
              //DS->dump();
              Decl *d = DS->getSingleDecl();
              /* if d is varDecl */ 

              int fn = func_name.length();
              char func_name_array[fn+1];
              strcpy(func_name_array, func_name.c_str());
              if ( VarDecl *V = dyn_cast<VarDecl>(d))
              {
                // VarDecl *V = cast<VarDecl>(d);
                /*insert in func_varDecl map*/
                func_varDecl_map[func_name].insert(V);
                
                string name = V->getNameAsString();
                int n = name.length();
                char name_array[n+1];
                strcpy(name_array, name.c_str());
                
                char structInitial[1000];
                if (V->getType().getTypePtr()->isConstantArrayType()) {
                  sprintf(structInitial, "\n\t%s_v.%s = %s;", func_name_array, name_array, name_array);
                }
                else if (V->getType().getTypePtr()->isStructureType()) {
                  sprintf(structInitial, "\n\t%s_v.%s = &%s;", func_name_array, name_array, name_array);
                }
                else {
                  sprintf(structInitial, "\n\t%s_v.%s = &%s;", func_name_array, name_array, name_array);
                }
                Rewrite.InsertTextAfterToken(s, structInitial);
              }
              else if (RecordDecl *R = dyn_cast<RecordDecl>(d)) 
              {
                // R->dump();
                /*Insert in func_recordDecl_map*/
                func_recordDecl_map[func_name].insert(R);
              }

            }
            else 
            {
              for(DeclStmt::decl_iterator it = DS->decl_begin(); it != DS->decl_end(); it++ ) {
                Decl *d = *it;
                //d->dump();
                VarDecl *V = cast<VarDecl>(d);
                /*insert in func_varDecl map*/
                func_varDecl_map[func_name].insert(V);
                
                string name = V->getNameAsString();
                int n = name.length();
                char name_array[n+1];
                strcpy(name_array, name.c_str());
                int fn = func_name.length();
                char func_name_array[fn+1];
                strcpy(func_name_array, func_name.c_str());
                char structInitial[256];
                if (V->getType().getTypePtr()->isConstantArrayType()) {
                  sprintf(structInitial, "\n\t%s_v.%s = %s;", func_name_array, name_array, name_array);
                }
                else {
                  sprintf(structInitial, "\n\t%s_v.%s = &%s;", func_name_array, name_array, name_array);
                }
                
                Rewrite.InsertTextAfterToken(s, structInitial);
              }
            }
          
          }
        }
      }
    }
    return true;
  }

  private:
  Rewriter &Rewrite;
};

class VarDeclHandler : public MatchFinder::MatchCallback {
  public:
    VarDeclHandler(Rewriter &Rewrite) : Rewrite(Rewrite) {}

    virtual void run(const MatchFinder::MatchResult &Result) {
      const VarDecl *V = Result.Nodes.getNodeAs<clang::VarDecl>("varDecl");
      if (V && !V->isLocalVarDecl()) {
        // if V is a global var
        func_varDecl_map["global_varDecl"].insert(V);
      }
    }

  private:
    Rewriter &Rewrite;
};

// Implementation of the ASTConsumer interface for reading an AST produced
// by the Clang parser. It registers a couple of matchers and runs them on
// the AST.
class MyASTConsumer : public ASTConsumer {
public:
  MyASTConsumer(Rewriter &R) : HandlerForVarDecl(R), HandlerForLabel(R), visitor(R) {
    Matcher.addMatcher(varDecl(
    isExpansionInMainFile()).bind("varDecl"), &HandlerForVarDecl);
    
    Matcher.addMatcher(labelStmt().bind("labelStmt"), &HandlerForLabel);
  }

  void HandleTranslationUnit(ASTContext &Context) override {  
    astContext = &Context;  
    visitor.TraverseDecl(Context.getTranslationUnitDecl());
    // Run the matchers when we have the whole TU parsed.
    Matcher.matchAST(Context); 
  }

private:
  VarDeclHandler HandlerForVarDecl;
  RewriteLabelStmtHandler HandlerForLabel;
  MyASTVisitor visitor;
  MatchFinder Matcher;
};

// For each source file provided to the tool, a new FrontendAction is created.
class MyFrontendAction : public ASTFrontendAction {
public:
  MyFrontendAction() {}

  SourceLocation getIncludeLocation() {
    SourceManager &sm = TheRewriter.getSourceMgr();
    FileID fileID = sm.getMainFileID();
    
    if (fileID.isInvalid())
      return SourceLocation();
    
    set<unsigned> lines;
    for (auto it = sm.fileinfo_begin(); it != sm.fileinfo_end(); it++) {
        SourceLocation includeLoc = sm.getIncludeLoc(sm.translateFile(it->first));
        if (includeLoc.isValid() && sm.isInFileID(includeLoc, fileID)) {
            lines.insert(sm.getSpellingLineNumber(includeLoc));
        }
    }
    return sm.translateFileLineCol(sm.getFileEntryForID(fileID), lines.size()+1, 1);
  }

  string _getType(const VarDecl *V) {
      QualType q = V->getType();
      const clang::Type *typ = q.getTypePtr();
      string type;
      if (typ->isIntegerType())
        type = "int";
      else if (typ->isRealFloatingType())
        type = "float";
      else if (typ->isConstantArrayType())
        type = "array";
      else if (typ->isStructureType())
        type = "struct";

      return type;      
  }

  void createVarStruct() 
  {
    for ( auto it = func_varDecl_map.begin(); it != func_varDecl_map.end(); it++ ) 
    {
      string func_name = it->first;
      if (func_name == "global_varDecl") {
        continue;
      }

        int count = it->second.size();
        if (count) 
        {
          // TheRewriter.InsertTextAfterToken(postIncludeLocation, "\n};\n");
          int func_name_len = func_name.length();
          char func_name_array[func_name_len+1];
          strcpy(func_name_array, func_name.c_str());
          char structDecl[256];
          sprintf(structDecl, "\nstruct %s_var {", func_name_array);
          TheRewriter.InsertText(postIncludeLocation.getLocWithOffset(-1), structDecl, true, true);

          for (auto jt = it->second.begin(); jt != it->second.end(); jt++) 
          {
            /* (*jt) is varDecl*/
            string name = (*jt)->getNameAsString();
            string typ = _getType(*jt);
            // llvm::outs() << type << " " << name << " ; ";
            if ( typ == "int" || typ == "float")
            {
              string type = (*jt)->getType().getAsString();
              int n = name.length();
              int t = type.length();
              char name_array[n+1], type_array[t+1];
              strcpy(name_array, name.c_str());
              strcpy(type_array, type.c_str());
              char varDeclaration[256];
              sprintf(varDeclaration, "\n\t%s* %s;", type_array, name_array);
              TheRewriter.InsertText(postIncludeLocation.getLocWithOffset(-1), varDeclaration, true, true);
            }
            else if ( typ == "array")
            {
              const clang::Type *typ_ptr = (*jt)->getType().getTypePtr();
              const clang::ArrayType *Array = typ_ptr->castAsArrayTypeUnsafe();
              string elementType = Array->getElementType().getAsString();
              int n = name.length();
              int t = elementType.length();
              char name_array[n+1], type_array[t+1];
              strcpy(name_array, name.c_str());
              strcpy(type_array, elementType.c_str());
              char varDeclaration[256];
              sprintf(varDeclaration, "\n\t%s* %s;", type_array, name_array);
              TheRewriter.InsertText(postIncludeLocation.getLocWithOffset(-1), varDeclaration, true, true);
            }
            else if ( typ == "struct" ) 
            {
              // llvm::outs() << "name =" << name << "\n";
              // llvm::outs() << "type = " << (*jt)->getType().getAsString() << "\n";
              size_t found = (*jt)->getType().getAsString().find_last_of(" ");
              string sname = (*jt)->getType().getAsString().substr(found+1);
              // llvm::outs() << "type = " << sname << "\n";
              // vector<pair< string, string > > fieldVec;
              int sn = sname.length();
              char sname_array[sn+1];
              strcpy(sname_array, sname.c_str());
              char structDeclaration[256];
              sprintf(structDeclaration, "\n\tstruct %s_%s {", func_name_array, sname_array);
              TheRewriter.InsertText(postIncludeLocation.getLocWithOffset(-1), structDeclaration, true, true);
              for (auto rit = func_recordDecl_map[func_name].begin(); rit != func_recordDecl_map[func_name].end(); rit++)
              {
                string rname = (*rit)->getNameAsString();
                // llvm::outs() << "record decl name = " << rname << " \n";
                if (sname == rname)
                {
                  for(RecordDecl::field_iterator fit = (*rit)->field_begin(); fit != (*rit)->field_end(); fit++ )
                  {
                    FieldDecl *fd = *fit;
                    string fname = fd->getNameAsString();
                    string ftype = fd->getType().getAsString();
                    // llvm::outs() << "field: " << fname << " , " << ftype << "\n";
                    int fn = fname.length();
                    int ft = ftype.length();
                    char fname_array[fn+1], ftype_array[ft+1];
                    strcpy(fname_array, fname.c_str());
                    strcpy(ftype_array, ftype.c_str());
                    char fieldDeclaration[256];
                    sprintf(fieldDeclaration, "\n\t\t%s %s;", ftype_array, fname_array);
                    TheRewriter.InsertText(postIncludeLocation.getLocWithOffset(-1), fieldDeclaration, true, true);
                  }
                }
              }
              int n = name.length();
              char name_array[n+1];
              strcpy(name_array, name.c_str());
              char varDeclaration[256];
              sprintf(varDeclaration, "\n\t} *%s;", name_array);
              TheRewriter.InsertText(postIncludeLocation.getLocWithOffset(-1), varDeclaration, true, true);

            }
          }

          TheRewriter.InsertText(postIncludeLocation.getLocWithOffset(-1), "\n};\n", true, true);

          int count_copy = count;
          string null = "NULL";
          string initialisation = "";
          initialisation += null;
          count_copy--;
          while(count_copy--) {
            initialisation += ", ";
            initialisation += null;
          }
          char structVarDecl[256];
          sprintf(structVarDecl, "\nstruct %s_var %s_v = { ", func_name_array, func_name_array);
          TheRewriter.InsertText(postIncludeLocation, structVarDecl, true, true);
          TheRewriter.InsertText(postIncludeLocation, initialisation , true, true);
          TheRewriter.InsertText(postIncludeLocation, " };\n", true, true);
        }
    }
  }

  void EndSourceFileAction() override 
  {
    postIncludeLocation = getIncludeLocation();
    createVarStruct();
    // for (auto it = func_varDecl_map.begin(); it != func_varDecl_map.end(); it++)
    // {
    //   llvm::outs() << (it->first)/*->getNameInfo().getName().getAsString()*/ << " ==> ";
    //   for (auto jt = it->second.begin(); jt != it->second.end(); jt++ )
    //   {
    //     string name = (*jt)->getNameAsString();
    //     string type = (*jt)->getType().getAsString();
    //     llvm::outs() << type << " " << name << " ; ";
    //   }
    //   llvm::outs() << "\n";
    // }

    TheRewriter.getEditBuffer(TheRewriter.getSourceMgr().getMainFileID())
        .write(llvm::outs());
  }

  std::unique_ptr<ASTConsumer> CreateASTConsumer(CompilerInstance &ci, StringRef file) override {
    CI = &ci;
    TheRewriter.setSourceMgr(ci.getSourceManager(), ci.getLangOpts());
    return std::make_unique<MyASTConsumer>(TheRewriter);
  }

private:
  Rewriter TheRewriter;
  SourceLocation postIncludeLocation;
};

int main(int argc, const char **argv) {
	//parse the command line arguments passed to your code.
	CommonOptionsParser OptionsParser(argc, argv, MyToolCategory);
	// create a new clang tool instance ( a libtooling environment)
	ClangTool Tool(OptionsParser.getCompilations(), OptionsParser.getSourcePathList());
	// run the clang tool, creating a new FrontendAction
	return Tool.run(newFrontendActionFactory<MyFrontendAction>().get());
}