#include "clang/AST/AST.h"
#include "clang/AST/ASTContext.h"
#include "clang/ASTMatchers/ASTMatchFinder.h"
#include "clang/ASTMatchers/ASTMatchers.h"
#include "clang/Basic/SourceManager.h"
#include "clang/Basic/TokenKinds.h"
#include "clang/Frontend/CompilerInstance.h"
#include "clang/Frontend/FrontendActions.h"
#include "clang/Lex/Lexer.h"
#include "clang/Rewrite/Core/Rewriter.h"
#include "clang/Tooling/CommonOptionsParser.h"
#include "clang/Tooling/Refactoring.h"
#include "clang/Tooling/Tooling.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/raw_ostream.h"
#include <iostream>
#include <sstream>
#include <stack>
#include <string>
#include <typeinfo>
#include <unordered_map>
#include <vector>
using namespace clang::SrcMgr;
using namespace clang::tooling;
using namespace clang::ast_matchers;
using namespace llvm;
using namespace clang;
using namespace std;
using namespace clang::driver;
//----------------------------------------------------global
// decls----------------------------------------------//
struct GlobalVars {
  std::string name;
  std::string type;
};

struct scope {
  std::string name;
  std::unordered_map<std::string, std::string> vars;
  std::unordered_map<std::string, int> locs;
};

struct bounds {
  int begin;
  int end;
};

std::vector<GlobalVars> globalVars;
std::vector<std::string> globalFuncDecls;
std::vector<std::string> labels;
std::unordered_map<std::string, std::string> parenChilMap;
std::unordered_map<std::string, std::string> structparenChilMap;
// scopes is for the scopes that need to be passed on into the functions.
std::unordered_map<std::string, scope> scopes;
// decls is for the decalrations that are inside a function or label stmt.
std::unordered_map<std::string, scope> decls;
// call rels is for resolution of calls to appropriate funcitons or labels.
std::unordered_map<std::string, std::string> callRels;
// visited labels is for the contructions of decls. marks the parent label stmts
// visited. in a new matcher this visited labels would not be considered.
std::unordered_map<std::string, bool> visitedLabels;
std::unordered_map<std::string, bool> visitedLabelsInRelBuilder;
// labelbounds has label bounds.
std::unordered_map<std::string, bounds> labelBounds;
std::unordered_map<std::string, bounds> structBounds;
// rewrttien bodies haves the bodies in the form of string of respective label
// stmt.
std::unordered_map<std::string, std::string> rewrittenBodies;
std::unordered_map<std::string, std::string> rewrittenBodiesFin;
// specialCalls is a vector which contains some calls which must be handeled
// spcially.
std::vector<std::string> specialCalls;
std::vector<std::string> specialLabels;
// record is a map of vectors whic contains record decl in a label or function.
std::unordered_map<std::string, std::vector<std::string>> record;
std::unordered_map<std::string, std::vector<std::string>> recordinrecord;
// found record is an unorderd map for found structure decls
std::unordered_map<std::string, bool> foundRecord;
std::unordered_map<std::string, bool> resolvedStruct;
// doNotRename is a map for structure declarations inside strructures
// so that they should not be renamed.
std::unordered_map<std::string, bool> doNotRename;
std::unordered_map<std::string, bool> doNotMoveThisStruct;
// parenstack is for matching parentheses
std::stack<char> parenStack;
// labelNameLenghts is present to remove the assist callresolver
// and contains the original labelname lengths.
std::unordered_map<std::string, int> labelNameLength;
// the below vector is of objects of location type to prevent rewriting
// of structure names multiple times.
std::vector<std::string> writtenLocs;

static llvm::cl::OptionCategory MyToolCategory("my-tool options");
static cl::extrahelp CommonHelp(CommonOptionsParser::HelpMessage);
static cl::extrahelp MoreHelp("\nMore help text...\n");

std::unordered_map<std::string, std::string> sourceLocs;
std::unordered_map<std::string, int> callDepths;

std::vector<std::pair<std::string, int>> depthSorted;
std::unordered_map<std::string, bool> structMade;
// renamedStruct is utility for structdump;
std::unordered_map<std::string, bool> renamedStruct;
std::unordered_map<std::string, std::string> newName;
std::unordered_map<std::string, bool> renamedWhileResolving;
// marked call depths is utility for finding call depths, marks all the calls as
// true. so they are not processed again.
std::unordered_map<std::string, bool> resolvedCalls;
// structtoemit is a replacement for structbuff
std::unordered_map<std::string, std::string> structToEmit;
DeclarationMatcher globalMatcher =
    varDecl(hasDeclContext(translationUnitDecl()), isExpansionInMainFile())
        .bind("global");
StatementMatcher labelMatcher =
    labelStmt(isExpansionInMainFile()).bind("label");
std::unordered_map<std::string, int> depths;
std::unordered_map<std::string, int> structdepths;
std::unordered_map<std::string, int> vardecldepths;
std::stringstream ss;
std::stringstream ss2;
std::stringstream structBuff;
//------------------------------------------------/global decls
// end-------------------------------------------//

//------------------------------------------------------logic to convert the
// label depths into a sorted list depthsorted.
typedef std::function<bool(std::pair<std::string, int>,
                           std::pair<std::string, int>)>
    Comparator;
Comparator compFunctor = [](std::pair<std::string, int> elem1,
                            std::pair<std::string, int> elem2) {
  return elem1.second < elem2.second;
};
//-----------------------------------------------------------------------------------------------------------------//
//---------------------------------------------------------------classes and
// functions begin---------------------//
void structDump() {
  // building the structs of scope in order of depth.
  for (auto label : depthSorted) {
    // run only for those functions whose depth is not zero and also
    // parenchilmap is not empty
    if (label.second != 0 && !parenChilMap[label.first].empty()) {
      ss << "struct s_" << label.first << " {\n";
      // add only if the depth of the parent is grater than 0.
      if (depths[parenChilMap[label.first]] != 0) {
        ss << "struct s_" << parenChilMap[label.first] << " *__s;\n";
      }
      for (auto var : scopes[label.first].vars) {
        // currently not adding any functionality to handle or structs any
        // differently.
        if (var.second.find('[') != std::string::npos) {
          // if the struct is renamed then use the new name here.
          if (var.second.find("struct") != std::string::npos &&
              renamedStruct[var.first + std::to_string(scopes[label.first]
                                                           .locs[var.first])]) {
            // check for any qualifiers in the var.second.
            std::string temp = var.second;
            temp = temp.substr(0, temp.find("struct"));
            if (!temp.empty()) {
              ss << temp << " ";
            }
						//if it is of structure type then add to it the number of '*'
						//present in its type.
						std::stringstream stars;
						stars.str("");
						if(var.second.find('*')!= std::string::npos){
							for(auto i = var.second.find_first_of('*'); i <= var.second.find_last_of('*')  ; i++){
								stars<<"*";	
							}
						}
            ss << "struct "
               << newName[var.first +
                          std::to_string(scopes[label.first].locs[var.first])]<<" "
               << stars.str()<<"* " << var.first << ";\n";
          } else {
            ss << var.second.substr(0, var.second.find('[')) << "* "
               << var.first << ";\n";
          }
        } else {
          // if var.second has struct in it and the struct is renamed then use
          // the new name not the old one. if the struct is renamed then use the
          // new name here.
          if (var.second.find("struct") != std::string::npos &&
              renamedStruct[var.first + std::to_string(scopes[label.first]
                                                           .locs[var.first])]) {
            // check for any qualifiers in the var.second.
            std::string temp = var.second;
            temp = temp.substr(0, temp.find("struct"));
            if (!temp.empty()) {
              ss << temp << " ";
            }
						//if it is of structure type then add to it the number of '*'
						//present in its type.
						std::stringstream stars;
						stars.str("");
						if(var.second.find('*')!= std::string::npos){
							for(auto i = var.second.find_first_of('*'); i <= var.second.find_last_of('*')  ; i++){
								stars<<"*";	
							}
						}
            ss << "struct "
               << newName[var.first +
                          std::to_string(scopes[label.first].locs[var.first])]<<" "
               << stars.str()<<"* " << var.first << ";\n";
          } else {
            ss << var.second << "* " << var.first << ";\n";
          }
        }
      }
      ss << "};\n\n";
    }
  }
  // add structs found in between to the ss.
  // donot add blindly add them in order of there source locations.
  int min;
  while (!structToEmit.empty()) {
    min = structBounds[structToEmit.begin()->first].begin;
    std::string target;
    for (auto limit : structToEmit) {
      if (structBounds[limit.first].begin <= min) {
        target = limit.first;
        min = structBounds[limit.first].begin;
      }
    }
    // cout<<target<<"min"<<min<<endl;
    ss << structToEmit[target];
    structToEmit.erase(target);
  }
  // ss << structBuff.str() << "";
  // print forward declarations of functions;
  for (auto label : labels) {
    ss << "void " << label << "( "
       << "struct s_" << label << "*);\n";
  }
  ss << "\n\n";
  // cout << ss.str();
}
//-------------------------------------------------------------------------------------------------------------//
void eraseAllSubStr(std::string &mainStr, std::string &toErase) {
  size_t pos = std::string::npos;

  // Search for the substring in string in a loop untill nothing is found
  while ((pos = mainStr.find(toErase)) != std::string::npos) {
    // If found then erase it from string
    mainStr.erase(pos, toErase.length());
  }
}
//-------------------------------------------------------------------------------------------------------------//
class GlobalBuilder : public MatchFinder::MatchCallback {
public:
  GlobalBuilder(Rewriter &Rewrite) : Rewrite(Rewrite) {}
  GlobalVars temp;

  virtual void run(const MatchFinder::MatchResult &Result) {
    if (const VarDecl *vd = Result.Nodes.getNodeAs<clang::VarDecl>("global")) {
      temp.type = vd->getType().getAsString();
      // llvm::errs()<<"globalbuilder\n";
      // llvm::errs()<<vd->getType().getAsString()<<"\n";
      temp.name = vd->getNameAsString();
      globalVars.push_back(temp);
    }
  }
  Rewriter &Rewrite;
};
//---------------------------------------------------------------------------------------------------------------//
class LabelFinder : public MatchFinder::MatchCallback {
public:
  LabelFinder(Rewriter &Rewrite) : Rewrite(Rewrite) {}
  virtual void run(const MatchFinder::MatchResult &Result) {
    SourceManager *sm = Result.SourceManager;
    if (const LabelStmt *ls =
            Result.Nodes.getNodeAs<clang::LabelStmt>("label")) {
      // llvm::errs()<<"labelfinder"<<ls->getName()<<"\n";
      std::string loc = ls->getBeginLoc().printToString(*sm);
      loc = loc.substr(loc.find(':') + 1, loc.find(':'));
      loc = loc.substr(0, loc.find(':'));
      sourceLocs[ls->getName() + loc] = loc;
      labels.push_back(ls->getName() + loc);
      //-----------------------------------------------------//
      std::string lsName = ls->getName();
      std::string lsloc = ls->getBeginLoc().printToString(*sm);
      lsloc = lsloc.substr(lsloc.find(':') + 1, lsloc.find(':'));
      lsloc = lsloc.substr(0, lsloc.find(':'));
      // findign the end label bounds
      std::string lsendloc = ls->getEndLoc().printToString(*sm);
      lsendloc = lsendloc.substr(lsendloc.find(':') + 1, lsendloc.find(':'));
      lsendloc = lsendloc.substr(0, lsendloc.find(':'));
      //-----------------------------------------------------//
      labelBounds[lsName + lsloc].begin = std::stoi(lsloc);
      labelBounds[lsName + lsloc].end = std::stoi(lsendloc);

      // initialize as !visitedLabels[lsName + lsloc] as false.
      visitedLabels[ls->getName() + loc] = false;
      // llvm::errs()<<loc<<"\n";
    }
  }
  Rewriter &Rewrite;
};
//---------------------------------------------------------------------------------------------------------------//
class GlobalStructRemover : public MatchFinder::MatchCallback {
public:
  GlobalStructRemover(Rewriter &R) : Rewrite(R) {}
  virtual void run(const MatchFinder::MatchResult &Result) {
    SourceManager *sm = Result.SourceManager;
    if (const RecordDecl *rdp =
            Result.Nodes.getNodeAs<clang::RecordDecl>("parent")) {
      if (const RecordDecl *rdc =
              Result.Nodes.getNodeAs<clang::RecordDecl>("child")) {
        std::string locrdc = rdc->getBeginLoc().printToString(*sm);
        if (locrdc.find("invalid") == std::string::npos) {
          locrdc = locrdc.substr(locrdc.find(':') + 1, locrdc.find(':'));
          locrdc = locrdc.substr(0, locrdc.find(':'));
          doNotMoveThisStruct[rdc->getNameAsString() + locrdc] = true;
        }
      }
    }
    if (const RecordDecl *rd =
            Result.Nodes.getNodeAs<clang::RecordDecl>("struct")) {
      std::string loclrd = rd->getBeginLoc().printToString(*sm);
      loclrd = loclrd.substr(loclrd.find(':') + 1, loclrd.find(':'));
      loclrd = loclrd.substr(0, loclrd.find(':'));
      // cout<<"xxxxxxxxxxxxxxxxxxxxxxxx"<<rd->getNameAsString()<<"
      // "<<loclrd<<endl;
      if (!doNotMoveThisStruct[rd->getNameAsString() + loclrd]) {
        // cout<<"yyyyyyyyyyyyyyyyyyyyyyyy"<<rd->getNameAsString()<<"
        // "<<loclrd<<endl;
        structToEmit[rd->getNameAsString() + loclrd] =
            Rewrite.getRewrittenText(rd->getSourceRange()) + ";\n\n";
        structBounds[rd->getNameAsString() + loclrd].begin = stoi(loclrd);
        Rewrite.ReplaceText(rd->getSourceRange(), "");
        Rewrite.ReplaceText(clang::Lexer::findNextToken(rd->getEndLoc(),
                                                        Rewrite.getSourceMgr(),
                                                        Rewrite.getLangOpts())
                                ->getLocation(),
                            "");
      }
    }
  }

private:
  Rewriter &Rewrite;
};
//---------------------------------------------------------------------------------------------------------------//
class RecordMatcher : public MatchFinder::MatchCallback {
public:
  RecordMatcher() {}
  virtual void run(const MatchFinder::MatchResult &Result) {
    SourceManager *sm = Result.SourceManager;
    if (const RecordDecl *rdp =
            Result.Nodes.getNodeAs<clang::RecordDecl>("parent")) {
      if (const RecordDecl *rdc =
              Result.Nodes.getNodeAs<clang::RecordDecl>("child")) {
        std::string loclrdp = rdp->getBeginLoc().printToString(*sm);
        loclrdp = loclrdp.substr(loclrdp.find(':') + 1, loclrdp.find(':'));
        loclrdp = loclrdp.substr(0, loclrdp.find(':'));

        std::string locrdc = rdc->getBeginLoc().printToString(*sm);
        if (locrdc.find("invalid") == std::string::npos) {
          locrdc = locrdc.substr(locrdc.find(':') + 1, locrdc.find(':'));
          locrdc = locrdc.substr(0, locrdc.find(':'));
          recordinrecord[rdp->getNameAsString() + loclrdp].push_back(
              rdc->getNameAsString() + locrdc);
          // push the record itself into the vector so that declaration
          // of same name resolves to it.
          recordinrecord[rdp->getNameAsString() + loclrdp].push_back(
              rdp->getNameAsString() + loclrdp);
          //      Rewrite.InsertTextAfterToken(
          //        clang::Lexer::findNextToken(rd->getBeginLoc(),
          //                                  Rewrite.getSourceMgr(),
          //                               Rewrite.getLangOpts())
          //      ->getLocation(),
          // locrd);
          //				structBuff<<Rewrite.getRewrittenText(rd->getSourceRange())<<";\n";
          // Rewrite.ReplaceText(rd->getBeginLoc(), "\n");
          // mark this record decl visited.
          foundRecord[rdc->getNameAsString() + locrdc] = true;
        }
      }
    }
    if (const LabelStmt *lp =
            Result.Nodes.getNodeAs<clang::LabelStmt>("parent")) {
      if (const RecordDecl *rd =
              Result.Nodes.getNodeAs<clang::RecordDecl>("child")) {
        std::string loclp = lp->getBeginLoc().printToString(*sm);
        loclp = loclp.substr(loclp.find(':') + 1, loclp.find(':'));
        loclp = loclp.substr(0, loclp.find(':'));

        std::string locrd = rd->getBeginLoc().printToString(*sm);
        if (locrd.find("invalid") == std::string::npos) {
          locrd = locrd.substr(locrd.find(':') + 1, locrd.find(':'));
          locrd = locrd.substr(0, locrd.find(':'));
          record[lp->getName() + loclp].push_back(rd->getNameAsString() +
                                                  locrd);
          //      Rewrite.InsertTextAfterToken(
          //        clang::Lexer::findNextToken(rd->getBeginLoc(),
          //                                  Rewrite.getSourceMgr(),
          //                               Rewrite.getLangOpts())
          //      ->getLocation(),
          // locrd);
          //				structBuff<<Rewrite.getRewrittenText(rd->getSourceRange())<<";\n";
          // Rewrite.ReplaceText(rd->getBeginLoc(), "\n");
          // mark this record decl visited.
          foundRecord[rd->getNameAsString() + locrd] = true;
        }
      }
    }
    if (const FunctionDecl *fd =
            Result.Nodes.getNodeAs<clang::FunctionDecl>("parent")) {
      if (const RecordDecl *rd =
              Result.Nodes.getNodeAs<clang::RecordDecl>("child")) {
        std::string loclp = fd->getBeginLoc().printToString(*sm);
        loclp = loclp.substr(loclp.find(':') + 1, loclp.find(':'));
        loclp = loclp.substr(0, loclp.find(':'));

        std::string locrd = rd->getBeginLoc().printToString(*sm);
        if (locrd.find("invalid") == std::string::npos) {
          locrd = locrd.substr(locrd.find(':') + 1, locrd.find(':'));
          locrd = locrd.substr(0, locrd.find(':'));
          if (!foundRecord[rd->getNameAsString() + locrd]) {
            record[fd->getNameAsString() + loclp].push_back(
                rd->getNameAsString() + locrd);
            //            Rewrite.InsertTextAfterToken(
            //              clang::Lexer::findNextToken(rd->getBeginLoc(),
            //                                        Rewrite.getSourceMgr(),
            //                                      Rewrite.getLangOpts())
            //            ->getLocation(),
            //      locrd);
            //	structBuff<<Rewrite.getRewrittenText(rd->getSourceRange())<<";\n";
            // Rewrite.ReplaceText(rd->getBeginLoc(), "\n");
            // mark this record decl visited.
            foundRecord[rd->getNameAsString() + locrd] = true;
          }
        }
      }
    }
  }
};
//---------------------------------------------------------------------------------------------------------------//
class RecordRewriter : public MatchFinder::MatchCallback {
public:
  RecordRewriter(Rewriter &Rewrite) : Rewrite(Rewrite) {}
  virtual void run(const MatchFinder::MatchResult &Result) {
    SourceManager *sm = Result.SourceManager;
    if (const RecordDecl *rdp =
            Result.Nodes.getNodeAs<clang::RecordDecl>("parent")) {
      if (const RecordDecl *rdc =
              Result.Nodes.getNodeAs<clang::RecordDecl>("child")) {

        std::string locrdc = rdc->getBeginLoc().printToString(*sm);
        locrdc = locrdc.substr(locrdc.find(':') + 1, locrdc.find(':'));
        locrdc = locrdc.substr(0, locrdc.find(':'));
        if (locrdc.find("invalid") == std::string::npos &&
            !foundRecord[rdc->getNameAsString() + locrdc] &&
            !doNotMoveThisStruct[rdc->getNameAsString() + locrdc]) {
          // cout << "rewriting record " << rd->getNameAsString() << " " <<
          // locrd
          //  << endl;
          std::string loc = clang::Lexer::findNextToken(rdc->getBeginLoc(),
                                                        Rewrite.getSourceMgr(),
                                                        Rewrite.getLangOpts())
                                ->getLocation()
                                .printToString(*sm);
          if (!renamedWhileResolving[loc]) {
            Rewrite.InsertTextAfterToken(
                clang::Lexer::findNextToken(rdc->getBeginLoc(),
                                            Rewrite.getSourceMgr(),
                                            Rewrite.getLangOpts())
                    ->getLocation(),
                locrdc);
          }
          // structBuff << Rewrite.getRewrittenText(rdc->getSourceRange()) <<
          // ";\n";
          // Rewrite.ReplaceText(rd->getBeginLoc(), "\n");
          // mark this record decl visited.
        }
        foundRecord[rdc->getNameAsString() + locrdc] = true;
      }
    }
    if (const LabelStmt *lp =
            Result.Nodes.getNodeAs<clang::LabelStmt>("parent")) {
      if (const RecordDecl *rd =
              Result.Nodes.getNodeAs<clang::RecordDecl>("child")) {

        std::string locrd = rd->getBeginLoc().printToString(*sm);
        locrd = locrd.substr(locrd.find(':') + 1, locrd.find(':'));
        locrd = locrd.substr(0, locrd.find(':'));
        // found record is true when struct is already rewritten.
        if (locrd.find("invalid") == std::string::npos &&
            !foundRecord[rd->getNameAsString() + locrd]) {
          // cout << "rewriting record " << rd->getNameAsString() << " " <<
          // locrd
          //  << endl;
          std::string loc = clang::Lexer::findNextToken(rd->getBeginLoc(),
                                                        Rewrite.getSourceMgr(),
                                                        Rewrite.getLangOpts())
                                ->getLocation()
                                .printToString(*sm);
          if (!renamedWhileResolving[loc]) {
            Rewrite.InsertTextAfterToken(
                clang::Lexer::findNextToken(rd->getBeginLoc(),
                                            Rewrite.getSourceMgr(),
                                            Rewrite.getLangOpts())
                    ->getLocation(),
                locrd);
          }
          structToEmit[rd->getNameAsString() + locrd] =
              Rewrite.getRewrittenText(rd->getSourceRange()) + ";\n\n";
          structBuff << Rewrite.getRewrittenText(rd->getSourceRange())
                     << ";\n\n";
          // Rewrite.ReplaceText(rd->getBeginLoc(), "\n");
          // mark this record decl visited.
        }
        foundRecord[rd->getNameAsString() + locrd] = true;
      }
    }
    if (const FunctionDecl *fd =
            Result.Nodes.getNodeAs<clang::FunctionDecl>("parent")) {
      if (const RecordDecl *rd =
              Result.Nodes.getNodeAs<clang::RecordDecl>("child")) {

        std::string locrd = rd->getBeginLoc().printToString(*sm);
        locrd = locrd.substr(locrd.find(':') + 1, locrd.find(':'));
        locrd = locrd.substr(0, locrd.find(':'));

        if (locrd.find("invalid") == std::string::npos) {
          if (!foundRecord[rd->getNameAsString() + locrd]) {
            // cout << "rewriting record " << rd->getNameAsString() << " " <<
            // locrd
            // << endl;
            std::string loc = clang::Lexer::findNextToken(
                                  rd->getBeginLoc(), Rewrite.getSourceMgr(),
                                  Rewrite.getLangOpts())
                                  ->getLocation()
                                  .printToString(*sm);
            if (!renamedWhileResolving[loc]) {
              Rewrite.InsertTextAfterToken(
                  clang::Lexer::findNextToken(rd->getBeginLoc(),
                                              Rewrite.getSourceMgr(),
                                              Rewrite.getLangOpts())
                      ->getLocation(),
                  locrd);
            }
            structToEmit[rd->getNameAsString() + locrd] =
                Rewrite.getRewrittenText(rd->getSourceRange()) + ";\n\n";
            structBuff << Rewrite.getRewrittenText(rd->getSourceRange())
                       << ";\n\n";
            // Rewrite.ReplaceText(rd->getBeginLoc(), "\n");
            // mark this record decl visited.
          }
        }
        foundRecord[rd->getNameAsString() + locrd] = true;
      }
    }
  }

private:
  Rewriter &Rewrite;
};
//---------------------------------------------------------------------------------------------------------------//
class RecordResolver : public MatchFinder::MatchCallback {
public:
  RecordResolver(Rewriter &Rewrite) : Rewrite(Rewrite) {}
  virtual void run(const MatchFinder::MatchResult &Result) {
    SourceManager *sm = Result.SourceManager;
    if (const RecordDecl *rdp =
            Result.Nodes.getNodeAs<clang::RecordDecl>("parent")) {
      if (const RecordDecl *rdc =
              Result.Nodes.getNodeAs<clang::RecordDecl>("child")) {
        // this is a struct recorddecl which is inside a structure
        // hence it should not be renamed.
        std::string rdcname = rdc->getNameAsString();
        std::string rdcloc = rdc->getBeginLoc().printToString(*sm);
        if (!rdcname.empty() && rdcloc.find("invalid") == std::string::npos) {
          rdcloc = rdcloc.substr(rdcloc.find(':') + 1, rdcloc.find(':'));
          rdcloc = rdcloc.substr(0, rdcloc.find(':'));
          // doNotRename[rdcname + rdcloc] = true;
          // cout << "donotrename record " << rdcname << " " << rdcloc << endl;
        }
      }
    }
    if (const RecordDecl *rd =
            Result.Nodes.getNodeAs<clang::RecordDecl>("parent")) {
      // find source loc of child
      std::string loclrd = rd->getBeginLoc().printToString(*sm);
      loclrd = loclrd.substr(loclrd.find(':') + 1, loclrd.find(':'));
      loclrd = loclrd.substr(0, loclrd.find(':'));

      if (const FieldDecl *fd =
              Result.Nodes.getNodeAs<clang::FieldDecl>("child")) {
        std::string vdName = fd->getNameAsString();
        std::string vdloc = fd->getBeginLoc().printToString(*sm);
        vdloc = vdloc.substr(vdloc.find(':') + 1, vdloc.find(':'));
        vdloc = vdloc.substr(0, vdloc.find(':'));

        // cout << "rewriting field decl:" << vdName << " " << vdloc <<"
        // "<<fd->getType().getAsString()<<endl;
        if (fd->getType().getUnqualifiedType().getAsString().find("struct") ==
            0) {
          resolvedStruct[vdName + vdloc] = true;
          // cout << "rewriting field decl:" << vdName << " " << vdloc <<" ";
          // extract only the structure name from the type.
          std::string type = fd->getType().getUnqualifiedType().getAsString();
          std::string structName = fd->getType()
                                       .getUnqualifiedType()
                                       .getBaseTypeIdentifier()
                                       ->getName()
                                       .str();
          // cout << "type " << type << endl;
          // cout<<type<<endl;
          if (type.find('[') == std::string::npos) {
            // this logic works because the type is always returned with
            // single spaaces.
            type.erase(std::remove(type.begin(), type.end(), ' '), type.end());
            // cout<<type<<endl;
            string res =
                findRecInRec(rd->getNameAsString() + loclrd, structName, vdloc);
            // id result is not 0 then at this point the vardecl can be
            // modified.
            // cout<<"res "<<res<<endl;
            if (res.compare("0") != 0) {
              auto loc = fd->getTypeSourceInfo()
                             ->getTypeLoc()
                             .getUnqualifiedLoc()
                             .getBeginLoc();
              loc = clang::Lexer::findNextToken(loc, Rewrite.getSourceMgr(),
                                                Rewrite.getLangOpts())
                        ->getLocation();
              // cout<<"var "<<loc.printToString(*sm)<<endl;
              if (std::find(writtenLocs.begin(), writtenLocs.end(),
                            loc.printToString(*sm)) == writtenLocs.end()) {
                Rewrite.ReplaceText(loc, res);
                writtenLocs.push_back(loc.printToString(*sm));
                renamedWhileResolving[loc.printToString(*sm)] = true;
              }
              // mark this struct as renamed for struct dump to correct name
              // while dumping.
              renamedStruct[vdName + vdloc] = true;
              newName[vdName + vdloc] = res;
            }
            // cout << res << "\n";
          } else {
            type.erase(std::remove(type.begin(), type.end(), ' '), type.end());
            string res =
                findRecInRec(rd->getNameAsString() + loclrd, structName, vdloc);
            // id result is not 0 then at this point the vardecl can be
            // modified.
            if (res.compare("0") != 0) {
              auto loc = fd->getTypeSourceInfo()
                             ->getTypeLoc()
                             .getUnqualifiedLoc()
                             .getBeginLoc();
              loc = clang::Lexer::findNextToken(loc, Rewrite.getSourceMgr(),
                                                Rewrite.getLangOpts())
                        ->getLocation();
              // cout<<"var "<<loc.printToString(*sm)<<endl;
              if (std::find(writtenLocs.begin(), writtenLocs.end(),
                            loc.printToString(*sm)) == writtenLocs.end()) {
                Rewrite.ReplaceText(loc, res);
                writtenLocs.push_back(loc.printToString(*sm));
                renamedWhileResolving[loc.printToString(*sm)] = true;
              }
              // mark this struct as renamed for struct dump to correct name
              // while dumping.
              renamedStruct[vdName + vdloc] = true;
              newName[vdName + vdloc] = res;
            }
            // cout << res << "\n";
          }
        }
      }
    }

    if (const LabelStmt *ls =
            Result.Nodes.getNodeAs<clang::LabelStmt>("parent")) {
      // find source loc of child
      std::string locls = ls->getBeginLoc().printToString(*sm);
      locls = locls.substr(locls.find(':') + 1, locls.find(':'));
      locls = locls.substr(0, locls.find(':'));
      if (const VarDecl *vd = Result.Nodes.getNodeAs<clang::VarDecl>("child")) {
        std::string vdName = vd->getNameAsString();
        std::string vdloc = vd->getBeginLoc().printToString(*sm);
        vdloc = vdloc.substr(vdloc.find(':') + 1, vdloc.find(':'));
        vdloc = vdloc.substr(0, vdloc.find(':'));
        if (vd->getType().getUnqualifiedType().getAsString().find("struct") ==
            0) {
          resolvedStruct[vdName + vdloc] = true;
          // cout << "rewriting struct decl:" << vdName << " " << vdloc << endl;
          // extract only the structure name from the type.
          std::string type = vd->getType().getUnqualifiedType().getAsString();
          std::string structName = vd->getType()
                                       .getUnqualifiedType()
                                       .getBaseTypeIdentifier()
                                       ->getName()
                                       .str();
          // cout << "type " << type << endl;
          // cout<<type<<endl;
          if (type.find('[') == std::string::npos) {
            // this logic works because the type is always returned with
            // single spaaces.
            type.erase(std::remove(type.begin(), type.end(), ' '), type.end());
            string res = findRec(ls->getName() + locls, structName, vdloc);
            // id result is not 0 then at this point the vardecl can be
            // modified.
            if (res.compare("0") != 0) {
              auto loc = vd->getTypeSourceInfo()
                             ->getTypeLoc()
                             .getUnqualifiedLoc()
                             .getBeginLoc();
              loc = clang::Lexer::findNextToken(loc, Rewrite.getSourceMgr(),
                                                Rewrite.getLangOpts())
                        ->getLocation();
              // cout<<"var "<<loc.printToString(*sm)<<endl;
              if (std::find(writtenLocs.begin(), writtenLocs.end(),
                            loc.printToString(*sm)) == writtenLocs.end()) {
                Rewrite.ReplaceText(loc, res);
                writtenLocs.push_back(loc.printToString(*sm));
                renamedWhileResolving[loc.printToString(*sm)] = true;
              }
              // mark this struct as renamed for struct dump to correct name
              // while dumping.
              renamedStruct[vdName + vdloc] = true;
              newName[vdName + vdloc] = res;
            }
            // cout << res << "\n";
          } else {
            type.erase(std::remove(type.begin(), type.end(), ' '), type.end());
            string res = findRec(ls->getName() + locls, structName, vdloc);
            // id result is not 0 then at this point the vardecl can be
            // modified.
            if (res.compare("0") != 0) {
              auto loc = vd->getTypeSourceInfo()
                             ->getTypeLoc()
                             .getUnqualifiedLoc()
                             .getBeginLoc();
              loc = clang::Lexer::findNextToken(loc, Rewrite.getSourceMgr(),
                                                Rewrite.getLangOpts())
                        ->getLocation();
              // cout<<"var "<<loc.printToString(*sm)<<endl;
              if (std::find(writtenLocs.begin(), writtenLocs.end(),
                            loc.printToString(*sm)) == writtenLocs.end()) {
                Rewrite.ReplaceText(loc, res);
                writtenLocs.push_back(loc.printToString(*sm));
                renamedWhileResolving[loc.printToString(*sm)] = true;
              }
              // mark this struct as renamed for struct dump to correct name
              // while dumping.
              renamedStruct[vdName + vdloc] = true;
              newName[vdName + vdloc] = res;
            }
            // cout << res << "\n";
          }
        }
      }
    }
    if (const FunctionDecl *ls =
            Result.Nodes.getNodeAs<clang::FunctionDecl>("parent")) {
      // find source loc of child
      std::string locls = ls->getBeginLoc().printToString(*sm);
      locls = locls.substr(locls.find(':') + 1, locls.find(':'));
      locls = locls.substr(0, locls.find(':'));
      if (const VarDecl *vd = Result.Nodes.getNodeAs<clang::VarDecl>("child")) {
        std::string vdName = vd->getNameAsString();
        std::string vdloc = vd->getBeginLoc().printToString(*sm);
        vdloc = vdloc.substr(vdloc.find(':') + 1, vdloc.find(':'));
        vdloc = vdloc.substr(0, vdloc.find(':'));
        if (vd->getType().getUnqualifiedType().getAsString().find("struct") ==
                0 &&
            resolvedStruct[vdName + vdloc] == false) {
          resolvedStruct[vdName + vdloc] = true;
          // cout << "rewriting struct decl:" << vdName << " " << vdloc << endl;
          std::string type = vd->getType().getUnqualifiedType().getAsString();
          std::string structName = vd->getType()
                                       .getUnqualifiedType()
                                       .getBaseTypeIdentifier()
                                       ->getName()
                                       .str();
          // cout << "type " << type << endl;
          // cout<<type<<endl;
          if (type.find('[') == std::string::npos) {
            type.erase(std::remove(type.begin(), type.end(), ' '), type.end());
            string res =
                findRec(ls->getNameAsString() + locls, structName, vdloc);
            // id result is not 0 then at this point the vardecl can be
            // modified.
            if (res.compare("0") != 0) {
              auto loc = vd->getTypeSourceInfo()
                             ->getTypeLoc()
                             .getUnqualifiedLoc()
                             .getBeginLoc();
              loc = clang::Lexer::findNextToken(loc, Rewrite.getSourceMgr(),
                                                Rewrite.getLangOpts())
                        ->getLocation();
              // cout<<"var "<<loc.printToString(*sm)<<endl;
              if (std::find(writtenLocs.begin(), writtenLocs.end(),
                            loc.printToString(*sm)) == writtenLocs.end()) {
                Rewrite.ReplaceText(loc, res);
                writtenLocs.push_back(loc.printToString(*sm));
                renamedWhileResolving[loc.printToString(*sm)] = true;
              }
              // mark this struct as renamed for struct dump to correct name
              // while dumping.
              renamedStruct[vdName + vdloc] = true;
              newName[vdName + vdloc] = res;
            }
            // cout << res << "\n";
          } else {
            type.erase(std::remove(type.begin(), type.end(), ' '), type.end());
            string res =
                findRec(ls->getNameAsString() + locls, structName, vdloc);
            // id result is not 0 then at this point the vardecl can be
            // modified.
            if (res.compare("0") != 0) {
              auto loc = vd->getTypeSourceInfo()
                             ->getTypeLoc()
                             .getUnqualifiedLoc()
                             .getBeginLoc();
              loc = clang::Lexer::findNextToken(loc, Rewrite.getSourceMgr(),
                                                Rewrite.getLangOpts())
                        ->getLocation();
              // cout<<"var "<<loc.printToString(*sm)<<endl;
              if (std::find(writtenLocs.begin(), writtenLocs.end(),
                            loc.printToString(*sm)) == writtenLocs.end()) {
                Rewrite.ReplaceText(loc, res);
                writtenLocs.push_back(loc.printToString(*sm));
                renamedWhileResolving[loc.printToString(*sm)] = true;
              }
              // mark this struct as renamed for struct dump to correct name
              // while dumping.
              renamedStruct[vdName + vdloc] = true;
              newName[vdName + vdloc] = res;
            }
            // cout << res << "\n";
          }
        }
      }
    }
  }
  std::string findRec(std::string ls, std::string target, std::string celoc) {
    // first find in the parent if a struct is present with the same name.
    if (depths[ls] == 0) {
      // find in the target in the list of the label ls.
      for (auto elem : record[ls]) {
        // cout << "elem: " << elem << " target: " << target << endl;
        if (elem.find(target) == 0 && structBounds[elem].begin <= stoi(celoc)) {
          return elem;
        }
      }
      return "0";
    }
    for (auto elem : record[ls]) {
      // cout << "elem: " << elem << " target: " << target << endl;
      if (elem.find(target) == 0 && structBounds[elem].begin <= stoi(celoc)) {
        return elem;
      }
    }
    return findRec(parenChilMap[ls], target, celoc);
  }
  std::string findRecInRec(std::string ls, std::string target,
                           std::string celoc) {
    // first find in the parent if a struct is present with the same name.
    if (structdepths[ls] == 0) {
      // find in the target in the list of the label ls.
      for (auto elem : recordinrecord[ls]) {
        // cout << "elem: " << elem << " target: " << target << endl;
        if (elem.find(target) == 0 && structBounds[elem].begin <= stoi(celoc)) {
          return elem;
        }
      }
      // if not found in the enclosing struct then go the parent of the ls
      // and then check the level 1 structure declarations only. will get parent
      // from parenchilmap in this case.
      // cout<<"xxxxxxxxxxxxxxxxxxxxxxxxxxx"<<ls<<"
      // "<<structparenChilMap[ls]<<endl;
      return findRecInParent(ls, target, celoc);
    }
    for (auto elem : recordinrecord[ls]) {
      // cout << "elem: " << elem << " target: " << target << endl;
      if (elem.find(target) == 0 && structBounds[elem].begin <= stoi(celoc)) {
        return elem;
      }
    }
    return findRecInRec(structparenChilMap[ls], target, celoc);
  }

  std::string findRecInParent(std::string ls, std::string target,
                              std::string celoc) {
    if (depths[ls] == 0) {
      // find in the target in the list of the label ls.
      for (auto elem : record[ls]) {
        // cout << "findrecinparentdepth0 elem: " << elem << " target: " <<
        // target <<"depth "<<structdepths[elem]<< endl;
        if (elem.find(target) == 0 && structdepths[elem] == 1 &&
            structBounds[elem].begin <= stoi(celoc)) {
          return elem;
        }
      }
      return "0";
    }
    for (auto elem : record[ls]) {
      // cout << "findrecinparent elem: " << elem << " target: " << target
      // <<"depth "<<structdepths[elem]<< endl;
      if (elem.find(target) == 0 && structdepths[elem] == 1 &&
          structBounds[elem].begin <= stoi(celoc)) {
        return elem;
      }
    }
    return findRecInParent(parenChilMap[ls], target, celoc);
  }

private:
  Rewriter &Rewrite;
};
//--------------------------------------------------------------------------------------------------------------------//
class LabelRelBuilder : public MatchFinder::MatchCallback {
public:
  LabelRelBuilder(Rewriter &Rewrite) : Rewrite(Rewrite) {}
  virtual void run(const MatchFinder::MatchResult &Result) {
    SourceManager *sm = Result.SourceManager;
    if (const LabelStmt *ls =
            Result.Nodes.getNodeAs<clang::LabelStmt>("child")) {
      // find source loc of child
      std::string locls = ls->getBeginLoc().printToString(*sm);
      locls = locls.substr(locls.find(':') + 1, locls.find(':'));
      locls = locls.substr(0, locls.find(':'));
      // add the source loc of child into sourceLocs.
      sourceLocs[ls->getName() + locls] = locls;
      // if parent is a labelstmt.
      if (const LabelStmt *lp =
              Result.Nodes.getNodeAs<clang::LabelStmt>("parent")) {
        // find source loc of parent.
        std::string loclp = lp->getBeginLoc().printToString(*sm);
        loclp = loclp.substr(loclp.find(':') + 1, loclp.find(':'));
        loclp = loclp.substr(0, loclp.find(':'));
        sourceLocs[lp->getName() + loclp] = loclp;
        parenChilMap[ls->getName() + locls] = lp->getName() + loclp;
        depths[ls->getName() + locls] = depths[lp->getName() + loclp] + 1;
        // if parent is a function decl.
      } else if (const FunctionDecl *fd =
                     Result.Nodes.getNodeAs<clang::FunctionDecl>("parent")) {
        std::string loclp = fd->getBeginLoc().printToString(*sm);
        loclp = loclp.substr(loclp.find(':') + 1, loclp.find(':'));
        loclp = loclp.substr(0, loclp.find(':'));
        sourceLocs[fd->getNameAsString() + loclp] = loclp;
        parenChilMap[ls->getName() + locls] = fd->getNameAsString() + loclp;
        // assuming that the nodes are visited in order of depth. starting from
        // lowest depth.
        depths[ls->getName() + locls] = 1;
        depths[fd->getNameAsString() + loclp] = 0;
        // inserting the length of the original function or label name
        // into the structure .
        std::string name = ls->getName();
        labelNameLength[ls->getName() + locls] = name.length();
        name = fd->getNameAsString();
        labelNameLength[fd->getNameAsString() + loclp] = name.length();
      }
    }
  }
  Rewriter &Rewrite;
};
//--------------------------------------------------------------------------------------------------------------------//
class StructRelBuilder : public MatchFinder::MatchCallback {
public:
  StructRelBuilder(Rewriter &Rewrite) : Rewrite(Rewrite) {}
  virtual void run(const MatchFinder::MatchResult &Result) {
    SourceManager *sm = Result.SourceManager;
    if (const RecordDecl *ls =
            Result.Nodes.getNodeAs<clang::RecordDecl>("child")) {
      // find source loc of child
      std::string locls = ls->getBeginLoc().printToString(*sm);
      locls = locls.substr(locls.find(':') + 1, locls.find(':'));
      locls = locls.substr(0, locls.find(':'));
      // if parent is a labelstmt.
      if (const RecordDecl *lp =
              Result.Nodes.getNodeAs<clang::RecordDecl>("parent")) {
        // find source loc of parent.
        std::string loclp = lp->getBeginLoc().printToString(*sm);
        loclp = loclp.substr(loclp.find(':') + 1, loclp.find(':'));
        loclp = loclp.substr(0, loclp.find(':'));
        structparenChilMap[ls->getNameAsString() + locls] =
            lp->getNameAsString() + loclp;
        structBounds[ls->getNameAsString() + locls].begin = stoi(locls);
        structdepths[ls->getNameAsString() + locls] =
            depths[lp->getNameAsString() + loclp] + 1;
        // if parent is a function decl.
      } else if (const FunctionDecl *fd =
                     Result.Nodes.getNodeAs<clang::FunctionDecl>("parent")) {
        std::string loclp = fd->getBeginLoc().printToString(*sm);
        loclp = loclp.substr(loclp.find(':') + 1, loclp.find(':'));
        loclp = loclp.substr(0, loclp.find(':'));
        structparenChilMap[ls->getNameAsString() + locls] =
            fd->getNameAsString() + loclp;
        structBounds[ls->getNameAsString() + locls].begin = stoi(locls);
        // assuming that the nodes are visited in order of depth. starting from
        // lowest depth.
        structdepths[ls->getNameAsString() + locls] = 1;
        structdepths[fd->getNameAsString() + loclp] = 0;
      } else if (const LabelStmt *fd =
                     Result.Nodes.getNodeAs<clang::LabelStmt>("parent")) {
        std::string loclp = fd->getBeginLoc().printToString(*sm);
        loclp = loclp.substr(loclp.find(':') + 1, loclp.find(':'));
        loclp = loclp.substr(0, loclp.find(':'));
        structparenChilMap[ls->getNameAsString() + locls] =
            fd->getName() + loclp;
        structBounds[ls->getNameAsString() + locls].begin = stoi(locls);
        // assuming that the nodes are visited in order of depth. starting from
        // lowest depth.
        structdepths[ls->getNameAsString() + locls] = 1;
        structdepths[fd->getName() + loclp] = 0;
      }
    }
  }
  Rewriter &Rewrite;
};
//-------------------------------------------------------------------------------------------------------------------------//
class StructBuilder : public MatchFinder::MatchCallback {
public:
  StructBuilder(Rewriter &Rewrite) : Rewrite(Rewrite) {}

  virtual void run(const MatchFinder::MatchResult &Result) {
    SourceManager *const sm = Result.SourceManager;
    if (const VarDecl *vd = Result.Nodes.getNodeAs<clang::VarDecl>("child")) {
      // find the appropriate parent child pair to insert it in. itereate
      // through parenChilMap and check if the source loc of the vardecl is in
      // between of parent and child.
      std::string loc = vd->getBeginLoc().printToString(*sm);
      if (loc.find("invalid") == std::string::npos) {
        loc = loc.substr(loc.find(':') + 1, loc.find(':'));
        loc = loc.substr(0, loc.find(':'));
        int varloc = std::stoi(loc);
        for (auto elem : parenChilMap) {
          if (std::stoi(sourceLocs[elem.first]) > varloc &&
              varloc >= std::stoi(sourceLocs[elem.second])) {
            // inserting into scopes for child label stmt
            // with name stype and scope.
            if(vardecldepths[vd->getNameAsString() + loc] == depths[elem.first]){
							scopes[elem.first].name = elem.first;
							scopes[elem.first].vars[vd->getNameAsString()] =
									vd->getType().getAsString();
							scopes[elem.first].locs[vd->getNameAsString()] = varloc;
							visitedLabels[elem.second] = true;
						}
          }
        }
      }
    }
  }
  Rewriter &Rewrite;
};
//------------------------------------------------------------------------find
// depths of all vardecl inside label statements------------//////

class VarDepthFinder : public MatchFinder::MatchCallback {
public:
  VarDepthFinder() {}
  virtual void run(const MatchFinder::MatchResult &Result) {
    SourceManager *const sm = Result.SourceManager;
    // initialize depths of all var decls as one.
    if (const VarDecl *vd = Result.Nodes.getNodeAs<clang::VarDecl>("one")) {
      std::string vdName = vd->getNameAsString();
      // llvm::errs()<<vdName<<"\n";
      std::string vdloc = vd->getBeginLoc().printToString(*sm);
      // llvm::errs() << vdloc << "\n";
      vdloc = vdloc.substr(vdloc.find(':') + 1, vdloc.find(':'));
      vdloc = vdloc.substr(0, vdloc.find(':'));
      // llvm::errs()<<vdloc<<"\n";
      if (vdloc.find("invalid") == std::string::npos) {
        vardecldepths[vdName + vdloc] = 1;
      }
    }
    // handle vardecs in label stmts.
    if (const VarDecl *vd = Result.Nodes.getNodeAs<clang::VarDecl>("child")) {
      if (const LabelStmt *ls =
              Result.Nodes.getNodeAs<clang::LabelStmt>("parent")) {
        std::string lsName = ls->getName();
        std::string lsloc = ls->getBeginLoc().printToString(*sm);
        lsloc = lsloc.substr(lsloc.find(':') + 1, lsloc.find(':'));
        lsloc = lsloc.substr(0, lsloc.find(':'));

        std::string vdName = vd->getNameAsString();
        std::string vdloc = vd->getBeginLoc().printToString(*sm);
        vdloc = vdloc.substr(vdloc.find(':') + 1, vdloc.find(':'));
        vdloc = vdloc.substr(0, vdloc.find(':'));
        if (vdloc.find("invalid") == std::string::npos) {
          vardecldepths[vdName + vdloc] = depths[lsName + lsloc] + 1;
        }
      }
    }
    // handle vardecls at global level.
    if (const VarDecl *vd = Result.Nodes.getNodeAs<clang::VarDecl>("child")) {
      if (const TranslationUnitDecl *tu =
              Result.Nodes.getNodeAs<clang::TranslationUnitDecl>("parent")) {

        std::string vdName = vd->getNameAsString();
        // llvm::errs()<<vdName<<"\n";
        std::string vdloc = vd->getSourceRange().getBegin().printToString(*sm);
        // llvm::errs()<<vdloc<<"\n";
        vdloc = vdloc.substr(vdloc.find(':') + 1, vdloc.find(':'));
        vdloc = vdloc.substr(0, vdloc.find(':'));
        if (vdloc.find("invalid") == std::string::npos) {
          vardecldepths[vdName + vdloc] = 0;
        }
      }
    }
  }
};
//---------------------------------------------------------------------------------------------------------------------//

//---------------------------------------------------------------------------------------------------------------------------//
class StructDumper : public MatchFinder::MatchCallback {
public:
  StructDumper(Rewriter &Rewrite) : Rewrite(Rewrite) {}
  virtual void run(const MatchFinder::MatchResult &Result) {
    if (const TranslationUnitDecl *tu =
            Result.Nodes.getNodeAs<clang::TranslationUnitDecl>("main")) {
      sourceLoc = tu->getASTContext().getSourceManager().getLocForStartOfFile(
          tu->getASTContext().getSourceManager().getMainFileID());
    }
  }
  virtual void onEndOfTranslationUnit() {
    depthSorted =
        std::vector<std::pair<std::string, int>>(depths.begin(), depths.end());
    std::sort(depthSorted.begin(), depthSorted.end(), compFunctor);
    structDump();
    Rewrite.InsertTextBefore(sourceLoc, ss.str());
  }

private:
  Rewriter &Rewrite;
  SourceLocation sourceLoc;
};
//-----------------------------------------------------------------------------------------------------------------------//

class CallDepth : public MatchFinder::MatchCallback {
public:
  CallDepth() {}
  virtual void run(const MatchFinder::MatchResult &Result) {
    SourceManager *const sm = Result.SourceManager;
    if (const CallExpr *ce =
            Result.Nodes.getNodeAs<clang::CallExpr>("depth1")) {
      std::string callName =
          ce->getDirectCallee()->getNameInfo().getName().getAsString();
      // llvm::errs() << "callee: " << callName << "\n";
      // find the source loc to find ce + sourceloc in labels vector.
      std::string loc = ce->getBeginLoc().printToString(*sm);
      loc = loc.substr(loc.find(':') + 1, loc.find(':'));
      loc = loc.substr(0, loc.find(':'));
      callDepths[callName + loc] = 1;
    } else if (const CallExpr *ce =
                   Result.Nodes.getNodeAs<clang::CallExpr>("depth")) {
      std::string callName =
          ce->getDirectCallee()->getNameInfo().getName().getAsString();
      // find source loc of call expr to find ce + sourceloc in labels vector.
      std::string loc = ce->getBeginLoc().printToString(*sm);
      loc = loc.substr(loc.find(':') + 1, loc.find(':'));
      loc = loc.substr(0, loc.find(':'));
      if (const LabelStmt *ls =
              Result.Nodes.getNodeAs<clang::LabelStmt>("parent")) {
        // find the loc of parent label to hash into the depths list.
        std::string lsloc = ls->getBeginLoc().printToString(*sm);
        lsloc = lsloc.substr(lsloc.find(':') + 1, lsloc.find(':'));
        lsloc = lsloc.substr(0, lsloc.find(':'));
        callDepths[callName + loc] = depths[ls->getName() + lsloc] + 1;
      }
    }
  }
};
//--------------------------------------------------------------------------------------------------------------------//
class GlobalFuncDeclFinder : public MatchFinder::MatchCallback {
public:
  GlobalFuncDeclFinder() {}
  virtual void run(const MatchFinder::MatchResult &Result) {
    // llvm::errs() << "global found.\n";
    if (const FunctionDecl *fd =
            Result.Nodes.getNodeAs<clang::FunctionDecl>("decl")) {
      if (fd->isGlobal()) {
        globalFuncDecls.push_back(fd->getNameAsString());
      }
    }
  }
};
//----------------------------------------------------------------------------------------------------------------//
class CallResolver : public MatchFinder::MatchCallback {
public:
  CallResolver() {}
  virtual void run(const MatchFinder::MatchResult &Result) {
    SourceManager *const sm = Result.SourceManager;
    if (const CallExpr *ce = Result.Nodes.getNodeAs<clang::CallExpr>("call")) {
      if (const LabelStmt *ls =
              Result.Nodes.getNodeAs<clang::LabelStmt>("parent")) {
        std::string callName =
            ce->getDirectCallee()->getNameInfo().getName().getAsString();
        std::string lsName = ls->getName();
        // once labelstmt name is known traverse all the parents recursively to
        // see if any labelstmt has the same name
        // as the callexpr. if i find match then stop and the call is resolved
        // to that label else if you reach a fucntion with depth zero then go on
        // to check if a global function is present or not.
        std::string lsloc = ls->getBeginLoc().printToString(*sm);
        lsloc = lsloc.substr(lsloc.find(':') + 1, lsloc.find(':'));
        lsloc = lsloc.substr(0, lsloc.find(':'));
        //--------------------------find loc of call-------------------//
        std::string celoc = ce->getBeginLoc().printToString(*sm);
        celoc = celoc.substr(celoc.find(':') + 1, celoc.find(':'));
        celoc = celoc.substr(0, celoc.find(':'));
        // check if there is a labestmt or functiondecl with same name as the
        // call expr at the same depth.
        // llvm::errs() << "helllo from call resolver\n";
        if (resolvedCalls[callName + celoc] == false) {
          std::string res = findCall(lsName + lsloc, callName, celoc);
          // llvm::errs() << callName + celoc << " " << res << "\n";
          // if 0 is returned then 2 cases are possible either the call resolves
          // to a global thing or it resolves to a label stmt at same depth.
          if (res.compare("0") == 0) {
            // search in parenchilmap for the label stmt that encloses this call
            // expression. if the label stmt has an immediate label stmt with
            // the same name then the call will resolve to it.
            // also one case was missing before here. if the call is to
            // a lable staatement that is at the same depth as the parent and
            // has source location less than it.
            //---------------------------------------------------------------//
            //							the call is
            // global
            ////
            //---------------------------------------------------------------//
            if (find(globalFuncDecls.begin(), globalFuncDecls.end(),
                     callName) != globalFuncDecls.end()) {
              // a global function call;
            } else {
              // throw error;
            }
          }
        }
      } else if (const FunctionDecl *fd =
                     Result.Nodes.getNodeAs<clang::FunctionDecl>("parent")) {
        // if call is not resolved here also then it should be checked for
        // global.
        std::string callName =
            ce->getDirectCallee()->getNameInfo().getName().getAsString();
        std::string celoc = ce->getBeginLoc().printToString(*sm);
        celoc = celoc.substr(celoc.find(':') + 1, celoc.find(':'));
        celoc = celoc.substr(0, celoc.find(':'));

        std::string fdName = fd->getNameAsString();
        std::string fdloc = fd->getBeginLoc().printToString(*sm);
        fdloc = fdloc.substr(fdloc.find(':') + 1, fdloc.find(':'));
        fdloc = fdloc.substr(0, fdloc.find(':'));
        // have name for both fd and ce check if it is unvisited
        if (resolvedCalls[callName + celoc] == false) {
          std::string res = findCall(fdName + fdloc, callName, celoc);
          // llvm::errs() << callName + celoc << " " << res << "\n";
          // if 0 is returned then 2 cases are possible either the call resolves
          // to a global thing or it resolves to a label stmt at same depth.
          if (res.compare("0") == 0) {
            // search in parenchilmap for the label stmt that encloses this call
            // expression. if the label stmt has an immediate label stmt with
            // the same name then the call will resolve to it. else it will
            // resolve to a global function.
            //-------------------------------------------------------------//
            //-------------------------------------------------------------//
            // the above explanation doesnt apply here if the paarent of a
            // call expr is a functions and it hasent been resolved yet then
            // it will surely resolve to the global function. the case of
            // checking at the same depth is not required here. Let it be
            // for a while.
            //-------------------------------------------------------------//
            //-------------------------------------------------------------//
            if (find(globalFuncDecls.begin(), globalFuncDecls.end(),
                     callName) != globalFuncDecls.end()) {
              // a global function call;
            } else {
              // throw error;
            }
          }
        }
      }
    }
  }

  std::string findCall(std::string ls, std::string target, std::string celoc) {
    std::string temp = ls;
    // removes digits from the name of the label after the name of the label.
    // NOTE::::::::::::::::::::A bug was present in which all digits were
    // removed. so if the label name had a digit it was also removed and the
    // was never removed.
    ls.erase(remove_if(ls.begin() + labelNameLength[ls], ls.end(),
                       [](char c) { return !isalpha(c); }),
             ls.end());
    if (depths[temp] == 0) {
      // only functions are at depth zero if the case below is not satsfied
      // then the call is must be to some other function which is global.
      if (ls.compare(target) == 0) {
        // if label or functiondecl at depth 0 is same as call then return it.
        // call resolution successfull mark true. else return 0 and the call
        // resolve to a global thing.
        resolvedCalls[target + celoc] = true;
        callRels[target + celoc] = temp;
        return temp;
      } else {
        // checking for the missing case. find the parent of the label stmt
        // which is a paren tof the label stmt.
        // check for all the label stmt with the same parent. if the name of
        // the label stmt is the same as of the call expr and sourcelocation
        // is less than the call then match found.
        for (auto label : labels) {
          // ls is the parent of the callexpr. and label is any label.
          if (parenChilMap[label].compare(temp) == 0) {
            // parent of both the labels is same check if the source loc is
            // less than the call expr and also its name is same as
            // callexpr.
            std::string tempLabel = label;
            tempLabel.erase(
                remove_if(tempLabel.begin() + labelNameLength[tempLabel],
                          tempLabel.end(), [](char c) { return !isalpha(c); }),
                tempLabel.end());
            // cout<<"incase1 "<<label<<" "<<labelBounds[label].begin<<"
            // "<<celoc<<endl;
            if (tempLabel.compare(target) == 0) {
              // the names are also same. check for location now.
              // the locs can also be equal in this case.
              if (labelBounds[label].begin <= stoi(celoc)) {
                // yeah matching call found.
                resolvedCalls[target + celoc] = true;
                callRels[target + celoc] = label;
                return label;
              }
            }
          }
        }
        // this is a global call.
        return "0";
      }
    }
    // check if the name of the label is same as the callee.
    if (ls.compare(target) == 0) {
      // call resolution is successfull mark true.
      resolvedCalls[target + celoc] = true;
      callRels[target + celoc] = temp;
      return temp;
    } else {
      // checking for the missing case. find the parent of the label stmt
      // which is a paren tof the label stmt.
      // check for all the label stmt with the same parent. if the name of
      // the label stmt is the same as of the call expr and sourcelocation
      // is less than the call then match found.
      for (auto label : labels) {
        // ls is the parent of the callexpr. and label is any label.
        if (parenChilMap[label].compare(parenChilMap[temp]) == 0) {
          // parent of both the labels is same check if the source loc is
          // less than the call expr and also its name is same as
          // callexpr.
          std::string tempLabel = label;
          tempLabel.erase(
              remove_if(tempLabel.begin() + labelNameLength[tempLabel],
                        tempLabel.end(), [](char c) { return !isalpha(c); }),
              tempLabel.end());
          // cout<<"incase3 templabel"<<tempLabel<<"
          // "<<labelBounds[label].begin<<" "<<celoc<<endl;
          if (tempLabel.compare(target) == 0) {
            // the names are also same. check for location now.
            // the locs can also be equal in this case.
            // cout<<"incase3 "<<label<<" "<<labelBounds[label].begin<<"
            // "<<celoc<<endl; here it was a bug earlier it was
            // labelBounds[ls].begin;
            if (labelBounds[label].begin <= stoi(celoc)) {
              // yeah matching call found.
              resolvedCalls[target + celoc] = true;
              callRels[target + celoc] = label;
              specialCalls.push_back(target + celoc);
              specialLabels.push_back(label);
              // cout<<"specialCall:"<<target + celoc;
              // cout<<"special label:"<<label<<"\n";
              return label;
            }
          }
        }
      }
      // now try to find in the children of the parent.
      for (auto elem : parenChilMap) {
        // temp is the current parent of the call.
        if (elem.second.compare(temp) == 0) {
          // the elem has the labelstmt as parent
          std::string tempLable = elem.first;
          // temp capture the name of child labelstmt;
          tempLable.erase(
              remove_if(tempLable.begin() + labelNameLength[tempLable],
                        tempLable.end(), [](char c) { return !isalpha(c); }),
              tempLable.end());
          if (tempLable.compare(target) == 0 &&
              labelBounds[elem.first].begin <= stoi(celoc)) {
            resolvedCalls[target + celoc] = true;
            callRels[target + celoc] = elem.first;
            return elem.first;
          }
        }
      }
      // if nothing found in the children of the parent of the label stmt
      // then go on to the next parent.
      return findCall(parenChilMap[temp], target, celoc);
    }
  }
};
//--------------------------------------------------------------------------------------------------------------------------//

class LabelLast : public MatchFinder::MatchCallback {
public:
  LabelLast() {}
  virtual void run(const MatchFinder::MatchResult &Result) {
    SourceManager *const sm = Result.SourceManager;
    if (const VarDecl *vd = Result.Nodes.getNodeAs<clang::VarDecl>("child")) {
      if (const LabelStmt *ls =
              Result.Nodes.getNodeAs<clang::LabelStmt>("parent")) {
        std::string lsName = ls->getName();
        std::string lsloc = ls->getBeginLoc().printToString(*sm);
        lsloc = lsloc.substr(lsloc.find(':') + 1, lsloc.find(':'));
        lsloc = lsloc.substr(0, lsloc.find(':'));
        // findign the end label bounds
        std::string lsendloc = ls->getEndLoc().printToString(*sm);
        lsendloc = lsendloc.substr(lsendloc.find(':') + 1, lsendloc.find(':'));
        lsendloc = lsendloc.substr(0, lsendloc.find(':'));
        // inserting the label bounds into the labelBounds.
        // llvm::errs() << lsName << " " << lsloc << " " << lsendloc << "\n";

        // llvm::errs() << "hello from lastlabel\n";
        lsloc = lsloc.substr(0, lsloc.find(':'));
        //  if (!visitedLabels[lsName + lsloc]) {
        // llvm::errs() << "yes\n";
        std::string loc = vd->getBeginLoc().printToString(*sm);
        loc = loc.substr(loc.find(':') + 1, loc.find(':'));
        loc = loc.substr(0, loc.find(':'));
        int varloc = std::stoi(loc);
        // llvm::errs() << lsName + lsloc << " " <<
        // vd->getQualifiedNameAsString()
        //             << "\n";
        // inserting into decl for the parent label stmt . usefull when re
        // writing. not to rewrite the vars that are declared in the
        // function. second is the parent first is child. marking the parent
        // as true visited.
        decls[lsName + lsloc].name = lsName + lsloc;
        decls[lsName + lsloc].vars[vd->getNameAsString()] =
            vd->getType().getAsString();
        decls[lsName + lsloc].locs[vd->getNameAsString()] = varloc;
      }
    }
  }
};
//--------------------------------------------------------------------------------------------------------------------------//
class StructInit : public MatchFinder::MatchCallback {
public:
  StructInit(Rewriter &Rewrite) : Rewrite(Rewrite) {}
  virtual void run(const MatchFinder::MatchResult &Result) {
    SourceManager *const sm = Result.SourceManager;
    if (const LabelStmt *ls =
            Result.Nodes.getNodeAs<clang::LabelStmt>("label")) {
      std::string lsloc = ls->getBeginLoc().printToString(*sm);
      lsloc = lsloc.substr(lsloc.find(':') + 1, lsloc.find(':'));
      lsloc = lsloc.substr(0, lsloc.find(':'));
      // start bulding string stream to insert structures ifthe label does not
      // resolve to a special call.
      // if (find(specialLabels.begin(), specialLabels.end(),
      //         ls->getName() + lsloc) == specialLabels.end()) {
      ss2 << "struct s_" << ls->getName() + lsloc << " s"
          << ls->getName() + lsloc << ";\n";
      // emit all the vars in the scope of that call.
      // add check to only emit if the var is not redefined in the
      // corresponding block.
      // the above comment is old and i have added the check to this
      // in the block when the variables are being rewritten in the
      // corresponding block.
      // the parent structure is also to be passed it is not in the
      // scopes struct it needs to be added manually.
      if (depths[ls->getName() + lsloc] != 1) {
        ss2 << "s" << ls->getName() + lsloc << ".__s = __s;\n";
      }
      for (auto var : scopes[ls->getName() + lsloc].vars) {
        // llvm::errs() << var.first << " " << var.second << " \n";
        ss2 << "s" << ls->getName() + lsloc << "." << var.first << " = &"
            << var.first << ";\n";
      }
      // llvm::errs() << ss2.str() << "\n";
      ss2 << ls->getName() + lsloc;
      Rewrite.ReplaceText(ls->getBeginLoc(), ss2.str());
      // Rewrite.InsertText(sourceLoc, ss2.str(), true, true);
      // Rewrite.InsertTextBefore(ce->getEndLoc(), "&s" + callName);
      ss2.str("");
      //}
    }

    if (const CallExpr *ce = Result.Nodes.getNodeAs<clang::CallExpr>("call")) {
      if (const LabelStmt *ls =
              Result.Nodes.getNodeAs<clang::LabelStmt>("parent")) {
        // bind the call expr and its parent label stmt.
        std::string callName =
            ce->getDirectCallee()->getNameInfo().getName().getAsString();
        std::string callLoc = ce->getBeginLoc().printToString(*sm);
        callLoc = callLoc.substr(callLoc.find(':') + 1, callLoc.find(':'));
        callLoc = callLoc.substr(0, callLoc.find(':'));
        // rewrite only those calls which are to label stmt.
        // callrels can be used here.
        if (callRels.find(callName + callLoc) != callRels.end()) {
          // if multiple call in a label stmt then only one call is to be
          // preceded by a structure definition.
          // if (structMade[ls->getName() + callName] == false) {
          // llvm::errs() << "entered init;\n";
          // if the call is of a label stmt and a struct for it has not
          // already been initialized. changed from structMade[lsName+ callLoc]
          // to structMade[callName + callLoc]. as the former one was always
          // false even for the call who have been initiliazed.
          structMade[callName + callLoc] = true;
          // find loc of labelstatement to hash into depths.
          std::string lsloc = ls->getBeginLoc().printToString(*sm);
          lsloc = lsloc.substr(lsloc.find(':') + 1, lsloc.find(':'));
          lsloc = lsloc.substr(0, lsloc.find(':'));
          // only those call are required to have the structs initialized
          // which are at depth of there parent.
          // llvm::errs() << callDepths[callName + callLoc] << " "
          //           << depths[callRels[callName + lsloc]] << "\n";
          // checking if the depths of the call and the label statement it
          // refers to is same.
          if (callDepths[callName + callLoc] ==
              depths[callRels[callName + callLoc]]) {
            // llvm::errs() << "inside final:\n";
            // sourceLoc = ce->getBeginLoc();
            // ss2 << "struct s_" << callRels[callName + callLoc] << " s"
            //    << callRels[callName + callLoc] + callLoc << ";\n";
            // emit all the vars in the scope of that call.
            // add check to only emit if the var is not redefined in the
            // corresponding block.
            // the above comment is old and i have added the check to this
            // in the block when the variables are being rewritten in the
            // corresponding block.
            // the parent structure is also to be passed it is not in the
            // scopes struct it needs to be added manually.
            // ss2 << "s" << callRels[callName + callLoc] + callLoc << ".s =
            // s;\n";

            // for (auto var : scopes[callRels[callName + callLoc]].vars) {
            // llvm::errs() << var.first << " " << var.second << " \n";
            // ss2 << "s" << callRels[callName + callLoc] + callLoc << "."
            //    << var.first << " = &" << var.first << ";\n";
            //}
            // llvm::errs() << ss2.str() << "\n";
            ss2 << callRels[callName + callLoc] << "(&s"
                << callRels[callName + callLoc] << ")";
            Rewrite.ReplaceText(ce->getSourceRange(), ss2.str());
            // Rewrite.InsertText(sourceLoc, ss2.str(), true, true);
            // Rewrite.InsertTextBefore(ce->getEndLoc(), "&s" + callName);
            ss2.str("");
          }
          // case when the depths are not same. only the cases which are valid
          // are the ones in which the calls are at depth greater than the
          // label they are referring to.
          else {
            // if it is a special call then handle differrently and add the
            // logic to dump structs before the call and also modify the call in
            // a different way.
            if (find(specialCalls.begin(), specialCalls.end(),
                     callName + callLoc) != specialCalls.end()) {
              ss2 << "struct s_" << callRels[callName + callLoc] << " s"
                  << callRels[callName + callLoc] + callLoc << ";\n";
              stringstream arrows;
              int diff = callDepths[callName + callLoc] -
                         depths[callRels[callName + callLoc]];
              // add diff number of arrows. the depth is surely more than the
              // call in special case.
              for (int i = 0; i < diff; i++) {
                arrows << "__s->";
              }
              // inserting structure initialization first.
              // add this line only if the depth of the resolved call is
              // greater than 1;
              if (depths[callRels[callName + callLoc]] > 1) {
                ss2 << "s" << callRels[callName + callLoc] + callLoc
                    << ".__s = " << arrows.str() << "__s;\n";
              }
              // add the variables in scope to ss2.
              for (auto var : scopes[callRels[callName + callLoc]].vars) {
                // llvm::errs() << var.first << " " << var.second << " \n";
                ss2 << "s" << callRels[callName + callLoc] + callLoc << "."
                    << var.first << " = " << arrows.str() + var.first << ";\n";
              }
              // Now variables are initialized put in the call
              ss2 << callRels[callName + callLoc] << "(&s"
                  << callRels[callName + callLoc] + callLoc << ")";
              arrows.str("");
            }
            // find the depth difference between the call and the label it
            // refers to.
            else {
              ss2 << callRels[callName + callLoc] << "(";
              int diff = callDepths[callName + callLoc] -
                         depths[callRels[callName + callLoc]];
              if (diff == 1) {
                // if difference is equal to one then just pass '__s'.
                ss2 << "__s";
              } else { // else when difference is not equal to one the pass a
                       // string.
                for (int i = 0; i < diff; i++) {
                  if (diff - i == 1) {
                    ss2 << "__s";
                  } else {
                    ss2 << "__s->";
                  }
                }
              }
              ss2 << ")";
            }
            Rewrite.ReplaceText(ce->getSourceRange(), ss2.str());
            // Rewrite.InsertTextBefore(ce->getEndLoc(), "&s" + callName);
            ss2.str("");
          }
          //}
        }
      }
    }

    if (const CallExpr *ce = Result.Nodes.getNodeAs<clang::CallExpr>("call")) {
      if (const FunctionDecl *fd =
              Result.Nodes.getNodeAs<clang::FunctionDecl>("parent")) {
        std::string callName =
            ce->getDirectCallee()->getNameInfo().getName().getAsString();
        std::string callLoc = ce->getBeginLoc().printToString(*sm);
        callLoc = callLoc.substr(callLoc.find(':') + 1, callLoc.find(':'));
        callLoc = callLoc.substr(0, callLoc.find(':'));
        // rewrite only those calls which are to label stmt.
        // callrels can be used here.
        if (callRels.find(callName + callLoc) != callRels.end()) {
          // this below condition will refine only the the level one calls
          // all other calls have been initilzed in the previous pass.
          if (structMade[callName + callLoc] == false) {
            // llvm::errs() << "entered init2;\n";
            // if the call is of a label stmt and a struct for it has not
            // already been initialized.
            // ----------------------------------------------------------//
            // dont do below line for level one functions--------------//

            //  structMade[fd->getNameAsString() + callName] = true;
            //------------------------------------------------------------//

            // find loc of labelstatement to hash into depths.
            std::string fdloc = fd->getBeginLoc().printToString(*sm);
            fdloc = fdloc.substr(fdloc.find(':') + 1, fdloc.find(':'));
            fdloc = fdloc.substr(0, fdloc.find(':'));
            // only those call are required to have the structs initialized
            // which are at depth of there parent.i
            // only those call are required to have the structs initialized
            // which are at depth of there parent.
            // llvm::errs() << callDepths[callName + callLoc] << " "
            //           << depths[callName + fdloc] << "\n";
            // checking if the depths of the call and the label statement it
            // refers to is same.
            if (callDepths[callName + callLoc] == 1) {
              // llvm::errs() << "inside final:\n";
              // sourceLoc = ce->getBeginLoc();
              // ss2 << "struct s_" << callRels[callName + callLoc] << " s"
              //    << callRels[callName + callLoc] + callLoc << ";\n";
              // emit all the vars in the scope of that call.
              // add check to only emit if the var is not redefined in the
              // corresponding block.
              // the above comment is old and i have added the check to this
              // in the block when the variables are being rewritten in the
              // corresponding block.
              // the parent structure is also to be passed it is not in the
              // scopes struct it needs to be added manually.

              // no need to use this line for level 1 functions.

              // ss2 << "s" << callRels[callName + callLoc] << ".s = s;\n";

              // for (auto var : scopes[callRels[callName + callLoc]].vars) {
              // llvm::errs() << var.first << " " << var.second << " \n";
              //  ss2 << "s" << callRels[callName + callLoc] + callLoc << "."
              //      << var.first << " = &" << var.first << ";\n";
              //}
              // llvm::errs() << ss2.str() << "\n";
              ss2 << callRels[callName + callLoc] << "(&s"
                  << callRels[callName + callLoc] << ")";
              Rewrite.ReplaceText(ce->getSourceRange(), ss2.str());
              // Rewrite.InsertText(sourceLoc, ss2.str(), true, true);
              // Rewrite.InsertTextBefore(ce->getEndLoc(), "&s" + callName);
              ss2.str("");
            }
            // case when the depths are not same. only the cases which are valid
            // are the ones in which the calls are at depth greater than the
            // label they are referring to.
            else {
              // find the depth difference between the call and the label it
              // refers to
              ss2 << callRels[callName + callLoc] << "(";
              int diff = callDepths[callName + callLoc] -
                         depths[callRels[callName + callLoc]];
              if (diff == 1) {
                // if difference is equal to one then just pass 's'.
                ss2 << "__s";
              } else { // else when difference is not equal to one the pass a
                       // string.
                for (int i = 0; i < diff; i++) {
                  if (diff - i == 1) {
                    ss2 << "__s";
                  } else {
                    ss2 << "__s->";
                  }
                }
              }
              ss2 << ")";
              Rewrite.ReplaceText(ce->getSourceRange(), ss2.str());
              // Rewrite.InsertTextBefore(ce->getEndLoc(), "&s" + callName);
              ss2.str("");
            }
          }
        }
      }
    }
  }

private:
  Rewriter &Rewrite;
  SourceLocation sourceLoc;
};

//--------------------------------------------------------------------------------------------------------------------------//

class LabelRewriter : public MatchFinder::MatchCallback {
public:
  LabelRewriter(Rewriter &Rewrite) : Rewrite(Rewrite) {}
  virtual void run(const MatchFinder::MatchResult &Result) {
    SourceManager *const sm = Result.SourceManager;
    if (const DeclRefExpr *dr =
            Result.Nodes.getNodeAs<clang::DeclRefExpr>("child")) {
      if (const LabelStmt *ls =
              Result.Nodes.getNodeAs<clang::LabelStmt>("parent")) {
        std::string lsname = ls->getName();
        std::string locls = ls->getBeginLoc().printToString(*sm);
        locls = locls.substr(locls.find(':') + 1, locls.find(':'));
        locls = locls.substr(0, locls.find(':'));
        // check in the decls structure of the label statement if a decl is
        // present in a line number less than or equal to the given declrefexpr.
        // if yes skip this declrexpr. if not recursively go on checking the
        // struct scopes of the parent and so on. if found in depth less than
        // zero then the number of drenferences is equal to the number of depth
        // of the label stmt in which it was found minus the depth of the parent
        // label stmt. first don't consider the declrefexpr which are funciton
        // calls.
        if (dr->getValueKind() != 0) {
          // drloc is declref location in code. drname is the decl
          // refname.
          std::string drloc = dr->getBeginLoc().printToString(*sm);
          drloc = drloc.substr(drloc.find(':') + 1, drloc.find(':'));
          drloc = drloc.substr(0, drloc.find(':'));
          std::string drname = dr->getNameInfo().getAsString();
          std::string drtype = dr->getType().getAsString();
          // llvm::errs() << drname << " " << drloc << " " << drtype << " ";

          // decloc is the location of the declaration correspondign to the use.
          // decname is the name of the declaratrion it is referring to,
          // although it would be the same.
          std::string decname = dr->getFoundDecl()->getNameAsString();
          std::string decloc =
              dr->getFoundDecl()->getBeginLoc().printToString(*sm);
          decloc = decloc.substr(decloc.find(':') + 1, decloc.find(':'));
          decloc = decloc.substr(0, decloc.find(':'));
          // llvm::errs() << decname << " " << decloc
          //             << vardecldepths[decname + decloc] << "\n";
          // now if the depth of the decalration is zero then nothing is to be
          // done.
          if (vardecldepths[decname + decloc] == 0) {
            // do nothing. global var used.
          }
          // if depth of the vardecl is same as the depth of use(use has depth
          // of labelstmt +1) and the declaration is before use then no need to
          // rewrite again.
          else if (vardecldepths[decname + decloc] ==
                       depths[lsname + locls] + 1 &&
                   stoi(decloc) <= stoi(drloc)) {
            // do nothing definition of declref is present in the block itself.
          }
          // if depth is less than the depth of the use then it is possibly from
          // some other struct. rewrite to add depthuse - depthdecl dereferences
          // on the struct.
          else if (vardecldepths[decname + decloc] <
                   depths[lsname + locls] + 1) {
            // first find the difference in depths;
            int diff =
                depths[lsname + locls] + 1 - vardecldepths[decname + decloc];
            // now indirections are to be handeled according to the dtype of the
            // of the variable.
            // llvm::errs() << drname << " " << drname.length() << "\n";
            stringstream ss;
            // add appropriate dereferences in the string.
            // handeling first for integers and floats.
            if ((drtype.find("int") != std::string::npos ||
                 drtype.find("float") != std::string::npos ||
                 drtype.find("struct") != std::string::npos) &&
                drtype.find("[") == std::string::npos) {
              ss << "(*(";
              for (int i = 0; i < diff; i++) {
                ss << "__s->";
              }
              Rewrite.InsertTextBefore(dr->getBeginLoc(), ss.str());
              Rewrite.InsertTextAfterToken(dr->getBeginLoc(), "))");
              ss.str("");
            }
            // it could be a typedef check it id the case.
            // if it is a array.
            else {
              // check the dimensionality of the array.
              int dims = std::count(drtype.begin(), drtype.end(), '[');
              if (dims == 1) {
                // find the index of the access
                // drloc =
                //    drloc.substr(drloc.find(':') + 1, drloc.find(':'));
                // drloc =
                // drloc.substr(0, drloc.find(':'));
                std::string inx =
                    drtype.substr(drtype.find('[') + 1, drtype.find(']'));
                ss << "(";
                for (int i = 0; i < diff; i++) {
                  ss << "__s->";
                }
                Rewrite.InsertTextBefore(dr->getBeginLoc(), ss.str());
                Rewrite.InsertTextAfterToken(dr->getBeginLoc(), ")");
                ss.str("");
              } else if (dims >= 2) {
                vector<std::string> arr;
                std::string type = dr->getType().getAsString();
                for (int i = 0; i < dims; i++) {
                  arr.push_back(type.substr(
                      type.find('[') + 1, type.find(']') - type.find('[') - 1));
                  type = type.substr(type.find(']') + 1,
                                     type.length() - type.find(']') - 1);
                  // cout << arr[i] << endl;
                }
                // things to insert before the array.
                ss << "(";
                for (int i = 0; i < diff; i++) {
                  ss << "__s->";
                }
                Rewrite.InsertTextBefore(dr->getBeginLoc(), ss.str());
                Rewrite.InsertTextAfterToken(dr->getBeginLoc(), ")");
                ss.str("");
                // iteratively get the location of the ']' so it can be used in
                // removed.
                // instead going to use stack based algorithm to remove the
                // approprite pair of parentheses.
                auto loc = dr->getBeginLoc();
                for (int i = 0; i < dims - 1; i++) {
                  while (1) {
                    // cout << "in while" << endl;
                    // cout << i << endl;
                    // cout << parenStack.size() << endl;
                    if (std::strcmp(clang::Lexer::findNextToken(
                                        loc, Rewrite.getSourceMgr(),
                                        Rewrite.getLangOpts())
                                        ->getName(),
                                    "l_square") == 0) {
                      // push the l_square into stack
                      parenStack.push('[');
                    } else if (std::strcmp(clang::Lexer::findNextToken(
                                               loc, Rewrite.getSourceMgr(),
                                               Rewrite.getLangOpts())
                                               ->getName(),
                                           "r_square") == 0) {
                      // there is always a matching parentheses so first pop and
                      // and then check if stack is empty.
                      if (!parenStack.empty()) {
                        parenStack.pop();
                      }
                      if (parenStack.empty()) {
                        break;
                      }
                    }
                    loc =
                        clang::Lexer::findNextToken(loc, Rewrite.getSourceMgr(),
                                                    Rewrite.getLangOpts())
                            ->getLocation();
                  }
                  // this loc is the loc of the last ']'.
                  loc = clang::Lexer::findNextToken(loc, Rewrite.getSourceMgr(),
                                                    Rewrite.getLangOpts())
                            ->getLocation();
                  // construct the string to place usin the dimensions.
                  stringstream inx;
                  for (int j = i + 1; j < dims; j++) {
                    inx << " * " << arr[j];
                  }
                  // append a '+' in last of string.
                  inx << " + ";
                  Rewrite.ReplaceText(loc, inx.str());
                  inx.str("");
                  // this loc is the loc of adjacent '['.
                  loc = clang::Lexer::findNextToken(loc, Rewrite.getSourceMgr(),
                                                    Rewrite.getLangOpts())
                            ->getLocation();
                  // push this ']' onto the stack for next iteration. otherwise
                  // parentheses will never match; the stack was already empty
                  // when it came out of the while loop.
                  parenStack.push('[');
                  Rewrite.ReplaceText(loc, "");
                }
              }
              // empty the stack for next match.
              while (!parenStack.empty()) {
                parenStack.pop();
              }
            } /* else if (drtype.find("float [") != std::string::npos) {
               // check the dims of array.
               int dims = std::count(drtype.begin(), drtype.end(), '[');
             }*/
          }
        }
      }
    }
  }

private:
  Rewriter &Rewrite;
};
//------------------------------------------------------------------------------------------------------------//
class LabelRenamer : public MatchFinder::MatchCallback {
public:
  LabelRenamer(Rewriter &Rewrite) : Rewrite(Rewrite) {}
  virtual void run(const MatchFinder::MatchResult &Result) {
    SourceManager *const sm = Result.SourceManager;
    if (const LabelStmt *ls =
            Result.Nodes.getNodeAs<clang::LabelStmt>("stmt")) {
      std::string lsname = ls->getName();
      std::string locls = ls->getBeginLoc().printToString(*sm);
      locls = locls.substr(locls.find(':') + 1, locls.find(':'));
      locls = locls.substr(0, locls.find(':'));
      Rewrite.ReplaceText(ls->getBeginLoc(), lsname + locls + "( struct s_" +
                                                 lsname + locls + "* s )");
    }
  }

private:
  Rewriter &Rewrite;
};
//------------------------------------------------------------------------------------------------------------//
class LabelRemover : public MatchFinder::MatchCallback {
public:
  LabelRemover(Rewriter &Rewrite) : Rewrite(Rewrite) {}
  virtual void run(const MatchFinder::MatchResult &Result) {
    SourceManager *const sm = Result.SourceManager;
    if (const LabelStmt *ls =
            Result.Nodes.getNodeAs<clang::LabelStmt>("stmt")) {
      std::string lsname = ls->getName();
      std::string locls = ls->getBeginLoc().printToString(*sm);
      locls = locls.substr(locls.find(':') + 1, locls.find(':'));
      locls = locls.substr(0, locls.find(':'));
      if (depths[lsname + locls] == 1) {
        Rewrite.ReplaceText(ls->getSourceRange(), "\n");
      }
    }
  }

private:
  Rewriter &Rewrite;
};
//-------------------------------------------------------------------------------------------------------------------------//
class LabelHoist : public MatchFinder::MatchCallback {
public:
  LabelHoist(Rewriter &Rewrite) : Rewrite(Rewrite) {}
  virtual void run(const MatchFinder::MatchResult &Result) {
    SourceManager *sm = Result.SourceManager;
    if (const LabelStmt *ls =
            Result.Nodes.getNodeAs<clang::LabelStmt>("child")) {
      // find source loc of child
      std::string locls = ls->getBeginLoc().printToString(*sm);
      locls = locls.substr(locls.find(':') + 1, locls.find(':'));
      locls = locls.substr(0, locls.find(':'));
      // if parent is a labelstmt.
      if (const LabelStmt *lp =
              Result.Nodes.getNodeAs<clang::LabelStmt>("parent")) {
        // find source loc of parent.
        std::string loclp = lp->getBeginLoc().printToString(*sm);
        loclp = loclp.substr(loclp.find(':') + 1, loclp.find(':'));
        loclp = loclp.substr(0, loclp.find(':'));
        // insert the rewritten body of the child label stmt into a
        // map hashed by the name and loc of the child labelstmt.
        rewrittenBodies[ls->getName() + locls] =
            Rewrite.getRewrittenText(ls->getSourceRange());
        rewrittenBodiesFin[ls->getName() + locls] =
            Rewrite.getRewrittenText(ls->getSourceRange());
        // cout << Rewrite.getRewrittenText(ls->getSourceRange()) << "\n";
        // if parent is a function decl.
      } else if (const FunctionDecl *fd =
                     Result.Nodes.getNodeAs<clang::FunctionDecl>("parent")) {
        std::string loclp = fd->getBeginLoc().printToString(*sm);
        loclp = loclp.substr(loclp.find(':') + 1, loclp.find(':'));
        loclp = loclp.substr(0, loclp.find(':'));
        // insert the rewritten body of the child label stmt into a
        // map hashed by the name and loc of the child labelstmt.
        rewrittenBodies[ls->getName() + locls] =
            Rewrite.getRewrittenText(ls->getSourceRange());
        rewrittenBodiesFin[ls->getName() + locls] =
            Rewrite.getRewrittenText(ls->getSourceRange());
        // cout << Rewrite.getRewrittenText(ls->getSourceRange()) << "\n";
      }
    }
  }
  Rewriter &Rewrite;
};
//---------------------------------------------------------------------------------------------------------------------------//
class FunctionDumper : public MatchFinder::MatchCallback {
public:
  FunctionDumper(Rewriter &Rewrite) : Rewrite(Rewrite) {}
  virtual void run(const MatchFinder::MatchResult &Result) {
    if (const TranslationUnitDecl *tu =
            Result.Nodes.getNodeAs<clang::TranslationUnitDecl>("main")) {
      sourceLoc = tu->getASTContext().getSourceManager().getLocForEndOfFile(
          tu->getASTContext().getSourceManager().getMainFileID());
    }
  }
  virtual void onEndOfTranslationUnit() {
    stringstream ss;
    ss << "\n\n";
    for (auto label : labels) {
      ss << rewrittenBodiesFin[label] << "\n\n";
    }
    Rewrite.InsertTextBefore(sourceLoc, ss.str());
    ss.str("");
  }

private:
  Rewriter &Rewrite;
  SourceLocation sourceLoc;
};
//-------------------------------------------------------------------------------------------------------------------------//
class MyASTConsumer : public ASTConsumer {
public:
  MyASTConsumer(Rewriter &R, PrintingPolicy &pp)
      : globalstructRemover(R), globalBuilder(R), labelBuilder(R),
        labelRelBuilder(R), structRelBuilder(R), structBuilder(R),
        structDumper(R), callDepth(), structInit(R), labelRewriter(R),
        labelRenamer(R), labelHoist(R), labelRemover(R), functionDumper(R),
        recordFinder(), recordResolver(R), recordRewriter(R) {
    GlobalStructFinder1.addMatcher(
        recordDecl(isExpansionInMainFile(),
                   hasDeclContext(translationUnitDecl()),
                   hasAncestor(recordDecl().bind("parent")))
            .bind("child"),
        &globalstructRemover);
    // finding global records and removing them appending to the structbuff;
    GlobalStructFinder2.addMatcher(
        recordDecl(isExpansionInMainFile(),
                   hasDeclContext(translationUnitDecl()))
            .bind("struct"),
        &globalstructRemover);
    // all code from main goes here.
    // Find all the globals and labelStmt first.
    // Find all the globals and store them in struct with type and identfier.
    Finder.addMatcher(globalMatcher, &globalBuilder);
    // find all the labelStmt and store there name in a vector of strings.
    Finder.addMatcher(labelMatcher, &labelBuilder);
    // below matcher returns all node at depth level one.
    // Finder.addMatcher(labelStmt(hasParent(compoundStmt(
    //                                hasParent(functionDecl().bind("parent")))))
    //                      .bind("child"),
    //                  &labelRelBuilder);
    Finder.addMatcher(labelStmt(hasAncestor(functionDecl().bind("parent")),
                                isExpansionInMainFile())
                          .bind("child"),
                      &labelRelBuilder);
    // find the parent child relationship of label statements by matching all
    // nodes having a compound statement as parent;
    Finder.addMatcher(labelStmt(hasAncestor(labelStmt().bind("parent")),
                                isExpansionInMainFile())
                          .bind("child"),
                      &labelRelBuilder);
    // finding the relations between nested structures.
    Finder.addMatcher(recordDecl(hasAncestor(functionDecl().bind("parent")),
                                 isExpansionInMainFile())
                          .bind("child"),
                      &structRelBuilder);
    // find the parent child relationship of label statements by matching all
    // nodes having a compound statement as parent;
    Finder.addMatcher(recordDecl(hasAncestor(labelStmt().bind("parent")),
                                 isExpansionInMainFile())
                          .bind("child"),
                      &structRelBuilder);
    // find the parent child relationship of label statements by matching all
    // nodes having a compound statement as parent;
    Finder.addMatcher(recordDecl(hasAncestor(recordDecl().bind("parent")),
                                 isExpansionInMainFile())
                          .bind("child"),
                      &structRelBuilder);
    // for all nodes find the variables that need to be passed on into its
    // scope.
    StructBuilderFinder.addMatcher(
        varDecl(hasAncestor(functionDecl()), isExpansionInMainFile())
            .bind("child"),
        &structBuilder);

    // Next thing would be to find the depths of the call expressions, so
    // appropriate redirections for variables may be provided. first
    // finding all calls at depth one.
    DelayedFinder.addMatcher(callExpr(hasAncestor(functionDecl()),
                                      argumentCountIs(0),
                                      isExpansionInMainFile())
                                 .bind("depth1"),
                             &callDepth);
    // finding the call expressions at more depths greater than .
    DelayedFinder.addMatcher(callExpr(hasAncestor(labelStmt().bind("parent")),
                                      argumentCountIs(0),
                                      isExpansionInMainFile())
                                 .bind("depth"),
                             &callDepth);
    // now the depths are in hand the logic to initialize scope srtuctures
    // before function calls must start. The below matcher inserts structs
    // intializations for function calls before the functions at a depth level
    // of one in label statements.
    //-------------------------------------------------------------------//
    // going to find global decls needed for call resolution
    DelayedFinder.addMatcher(
        functionDecl(hasParent(translationUnitDecl()), isExpansionInMainFile())
            .bind("decl"),
        &globalFuncDeclFinder);
    // first going to resolve calls at depth greatewr than 1 before the
    // strutcs are emitted.
    MatchCallResolver.addMatcher(
        callExpr(hasAncestor(labelStmt().bind("parent")), argumentCountIs(0),
                 isExpansionInMainFile())
            .bind("call"),
        &callResolver);
    // resolve calls at depths 1
    MatchCallResolver2.addMatcher(
        callExpr(hasAncestor(functionDecl().bind("parent")), argumentCountIs(0),
                 isExpansionInMainFile())
            .bind("call"),
        &callResolver);
    // now finding the vars in the last level label.
    // first finding out the last label stmts. all other label stmts
    // have been marked in some previous matcher.
    // below matcher just places finds the startloc and endloc of structs.
    // LastLabelFinder.addMatcher(labelStmt().bind("label"), &labelLast);
    // need to build decls for the label stmt at innermost depth.
    LastLabelFinder.addMatcher(varDecl(hasAncestor(labelStmt().bind("parent")),
                                       isExpansionInMainFile())
                                   .bind("child"),
                               &labelLast);
    // finding depths for vardecls to prune from structures.
    VarDepthFinderFinder.addMatcher(varDecl(isExpansionInMainFile()).bind("one"),
                              &varDepthFinder);
    // for decls within labels
    VarDepthFinderFinder.addMatcher(varDecl(hasAncestor(labelStmt().bind("parent")),
                                      isExpansionInMainFile())
                                  .bind("child"),
                              &varDepthFinder);
    // for decls at global level
    VarDepthFinderFinder.addMatcher(
        varDecl(hasDeclContext(translationUnitDecl().bind("parent")),
                isExpansionInMainFile())
            .bind("child"),
        &varDepthFinder);
    // find the relationships between the record decls and label stmts.
    // Also between recorddecls inside record decls.
    RecordFinder.addMatcher(recordDecl(hasAncestor(recordDecl().bind("parent")),
                                       isExpansionInMainFile())
                                .bind("child"),
                            &recordFinder);
    RecordFinder.addMatcher(recordDecl(hasAncestor(labelStmt().bind("parent")),
                                       isExpansionInMainFile())
                                .bind("child"),
                            &recordFinder);
    RecordFinder2.addMatcher(
        recordDecl(hasAncestor(functionDecl().bind("parent")),
                   isExpansionInMainFile())
            .bind("child"),
        &recordFinder);
    // first mark all the vardecl having a record decl as ancestor. the cases
    // are not to be rewritten as they are inside the structures themselves.
    // RecordFinder0.addMatcher(
    //    varDecl(hasAncestor(recordDecl().bind("parent"))).bind("child"),
    //    &recordResolver);
    // then mark all recorddecl inside record decl so that they are  also not
    // renamed.
    // RecordFinder0.addMatcher(
    //    recordDecl(hasAncestor(recordDecl().bind("parent"))).bind("child"),
    //    &recordResolver);
    // going to rewrite the vardecls of structs ignoring the ones not to
    // be rewritten.
    RecordFinder3.addMatcher(
        fieldDecl(
            hasAncestor(recordDecl(isExpansionInMainFile()).bind("parent")))
            .bind("child"),
        &recordResolver);
    RecordFinder3.addMatcher(
        varDecl(hasAncestor(labelStmt(isExpansionInMainFile()).bind("parent")))
            .bind("child"),
        &recordResolver);
    RecordFinder4.addMatcher(
        varDecl(
            hasAncestor(functionDecl(isExpansionInMainFile()).bind("parent")))
            .bind("child"),
        &recordResolver);
    // now rewrite the struct definitions by appending the source loc after
    // their name also append the structs which are to be hoisted to struct
    // stream.
    // also recorddecl inside recorddecl must be renamed because when hoisted
    // to global level they give name conflicts.
    RecordRewriteFinder0.addMatcher(
        recordDecl(hasAncestor(recordDecl().bind("parent")),
                   isExpansionInMainFile())
            .bind("child"),
        &recordRewriter);
    RecordRewriteFinder.addMatcher(
        recordDecl(hasAncestor(labelStmt().bind("parent")),
                   isExpansionInMainFile())
            .bind("child"),
        &recordRewriter);
    RecordRewriteFinder1.addMatcher(
        recordDecl(hasAncestor(functionDecl().bind("parent")),
                   isExpansionInMainFile())
            .bind("child"),
        &recordRewriter);
    // rewitrefinder will try to find all the vardecl and try to get  replace
    // text at appropriate places.
    RewriteFinder.addMatcher(
        declRefExpr(hasAncestor(labelStmt().bind("parent"))).bind("child"),
        &labelRewriter);
    // Use delayedfinder2 from this point and beyond
    // after the call to structDumper the depths will be sorted. structDumper
    // has a logic to sort the depths. adding code to find the main function
    // and rewrite something there.
    // first the calls at depths greater than one have there structs intitalized
    // then the ones at depth one.
    DelayedFinder2.addMatcher(labelStmt().bind("label"), &structInit);
    DelayedFinder2.addMatcher(
        callExpr(hasAncestor(labelStmt().bind("parent"))).bind("call"),
        &structInit);
    // now dump the structures they will be right.
    MainDumpFinder.addMatcher(translationUnitDecl().bind("main"),
                              &structDumper);
    // have to add a new matcher to add structures and call before functions
    // call from any function decl. to do this in the ast matcher bind the
    // nodes parent function decl and if it is the actual parent of the label
    // corresponding to the call.
    DelayedFinder3.addMatcher(
        callExpr(hasAncestor(functionDecl().bind("parent"))).bind("call"),
        &structInit);
    // renmae label statements.
    LabelRenameFinder.addMatcher(labelStmt().bind("stmt"), &labelRenamer);
    // label hoisting begins
    LabelHoistFinder.addMatcher(
        labelStmt(hasAncestor(functionDecl().bind("parent"))).bind("child"),
        &labelHoist);
    LabelHoistFinder.addMatcher(
        labelStmt(hasAncestor(labelStmt().bind("parent"))).bind("child"),
        &labelHoist);
    // remove label stmts from the code.
    LabelRemoveFinder.addMatcher(labelStmt().bind("stmt"), &labelRemover);
    FunctionDumpFinder.addMatcher(translationUnitDecl().bind("main"),
                                  &functionDumper);
  }

  void HandleTranslationUnit(ASTContext &Context) override {
    // Run the matchers when we have the whole TU parsed.
    GlobalStructFinder1.matchAST(Context);
    GlobalStructFinder2.matchAST(Context);
    Finder.matchAST(Context);
    DelayedFinder.matchAST(Context);
		MatchCallResolver.matchAST(Context);
    MatchCallResolver2.matchAST(Context);
    LastLabelFinder.matchAST(Context);
    VarDepthFinderFinder.matchAST(Context);
	  StructBuilderFinder.matchAST(Context); 
    //--------------------------------------------------------------------------------------------------//
    // prune the structures to remove invalid variables being passed.
    vector<std::string> toRemove;
    for (auto &label : labels) {
      for (auto &var : scopes[label].vars) {
        if (vardecldepths[var.first +
                          to_string(scopes[label].locs[var.first])] >
            depths[label]) {
          toRemove.push_back(var.first);
        }
      }
			if(!toRemove.empty()){
				for (auto key : toRemove) {
					scopes[label].vars.erase(key);
				}
				toRemove.clear();
			}
    }
    //---------------------------------------------------------------------------------------------------------------//
    RecordFinder.matchAST(Context);
    RecordFinder2.matchAST(Context);
    RecordFinder0.matchAST(Context);
    RecordFinder3.matchAST(Context);
    RecordFinder4.matchAST(Context);
    //---------------------------------for rewriting labels//
    RewriteFinder.matchAST(Context);
    //-----------------------------------------------------//
    // reset foundRecord as it is reused in rewriting.
    foundRecord.clear();
    RecordRewriteFinder0.matchAST(Context);
    RecordRewriteFinder.matchAST(Context);
    RecordRewriteFinder1.matchAST(Context);
    DelayedFinder2.matchAST(Context);
    DelayedFinder3.matchAST(Context);

    //----------------------------------------------------- dump the
    // structtures.
    MainDumpFinder.matchAST(Context);
    //----------------------------------------------------- rename all the label
    // statements.
    //    LabelRenameFinder.matchAST(Context);
    //    LabelHoistFinder.matchAST(Context);
    //-----------------------------------------now we can trim down the strings
    // in random order.
    for (auto label : labels) {
      for (auto elem : parenChilMap) {
        if (label == elem.second) {
          // match found, do trimming of strings.
          size_t pos =
              rewrittenBodiesFin[elem.second].find(rewrittenBodies[elem.first]);
          rewrittenBodiesFin[label].erase(pos,
                                          rewrittenBodies[elem.first].length());
        }
      }
    }
  }

private:
  GlobalStructRemover globalstructRemover;
  GlobalBuilder globalBuilder;
  LabelFinder labelBuilder;
  LabelRelBuilder labelRelBuilder;
  StructRelBuilder structRelBuilder;
  StructBuilder structBuilder;
  StructDumper structDumper;
  CallDepth callDepth;
  StructInit structInit;
  CallResolver callResolver;
  GlobalFuncDeclFinder globalFuncDeclFinder;
  LabelLast labelLast;
  VarDepthFinder varDepthFinder;
  LabelRewriter labelRewriter;
  LabelRenamer labelRenamer;
  LabelHoist labelHoist;
  LabelRemover labelRemover;
  FunctionDumper functionDumper;
  RecordMatcher recordFinder;
  RecordResolver recordResolver;
  RecordRewriter recordRewriter;
  MatchFinder Finder;
  MatchFinder MainDumpFinder;
  MatchFinder RecordFinder;
  MatchFinder RecordFinder0;
  MatchFinder RecordFinder01;
  MatchFinder RecordFinder2;
  MatchFinder RecordFinder3;
  MatchFinder RecordFinder4;
  MatchFinder RecordRewriteFinder0;
  MatchFinder RecordRewriteFinder;
  MatchFinder RecordRewriteFinder1;
  MatchFinder DelayedFinder;
  MatchFinder DelayedFinder2;
  MatchFinder DelayedFinder3;
  MatchFinder MatchCallResolver;
  MatchFinder MatchCallResolver2;
  MatchFinder LastLabelFinder;
  MatchFinder VarDepthFinderFinder;
  MatchFinder RewriteFinder;
  MatchFinder LabelRenameFinder;
  MatchFinder LabelHoistFinder;
  MatchFinder LabelRemoveFinder;
  MatchFinder FunctionDumpFinder;
  MatchFinder GlobalStructFinder1;
  MatchFinder GlobalStructFinder2;
	MatchFinder StructBuilderFinder;
};
//----------------------------------------------------------------------------------------------------------------------//
class MyFrontendAction : public ASTFrontendAction {
public:
  MyFrontendAction() {}
  Rewriter TheRewriter;

  void EndSourceFileAction() override {
    this->TheRewriter
        .getEditBuffer(this->TheRewriter.getSourceMgr().getMainFileID())
        .write(llvm::outs());
  }
  virtual std::unique_ptr<clang::ASTConsumer>
  CreateASTConsumer(clang::CompilerInstance &Compiler,
                    llvm::StringRef InFile) override {
    this->TheRewriter.setSourceMgr(Compiler.getSourceManager(),
                                   Compiler.getLangOpts());
    PrintingPolicy printingPolicy(Compiler.getLangOpts());
    return std::unique_ptr<clang::ASTConsumer>(
        new MyASTConsumer(this->TheRewriter, printingPolicy));
  }
};
//------------------------------------------------------------------------main
// starts--------------------------------//

int main(int argc, const char **argv) {
  CommonOptionsParser OptionsParser(argc, argv, MyToolCategory);
  ClangTool Tool(OptionsParser.getCompilations(),
                 OptionsParser.getSourcePathList());
  Tool.run(newFrontendActionFactory<MyFrontendAction>().get());
  return 0;
}
