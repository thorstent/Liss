/*
 * Copyright 2015, IST Austria
 *
 * This file is part of Liss.
 *
 * Liss is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Liss is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with Liss.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <clang/Frontend/CompilerInstance.h>
#include <clang/Frontend/FrontendAction.h>
#include <clang/Tooling/Tooling.h>
#include <clang/Tooling/CommonOptionsParser.h>

#include "llvm/Support/CommandLine.h" 

#include "clang_interf/abstraction_consumer.h"

#include <iostream>
#include "actions/action_base.h"
#include <vector>
#include "options.h"
#include <boost/filesystem.hpp>
#include "GIT.h"

using namespace clang;
using namespace std;
using namespace clang::tooling;
using namespace llvm;

// Apply a custom category to all command-line options so that they are the
// only ones displayed.
static llvm::cl::OptionCategory LissCategory("langinc options");

cl::list<actions::action_names> ActionList(cl::desc("Available Actions:"),
                                cl::values(
                                  clEnumValN(actions::action_names::print, "print"               , "Print automata"),
                                  clEnumValN(actions::action_names::printcfg, "printcfg"               , "Print control flow graph"),
                                  clEnumValN(actions::action_names::printtim, "printtim"               , "Print automata in timbuk format"),
                                  clEnumValN(actions::action_names::printthreads, "printthreads"       , "Print only threads (for large programs)"),
                                  clEnumValN(actions::action_names::inclusion_test, "inclusion"  , "Test inclusion of concurrent in sequential"),
                                  clEnumValN(actions::action_names::synthesis, "synthesis"  , "Add synchronisation primitives to the code"),
                                  clEnumValN(actions::action_names::deadlock, "deadlock"  , "Check for deadlocks"),
                                  clEnumValN(actions::action_names::perf_test, "perf"  , "Verious performance tests"),
                                  clEnumValN(actions::action_names::printlocks, "printlocks"  , "Print all lock positions in the code"),
                                clEnumValEnd), cl::cat(LissCategory));

cl::opt<int, true> Verbosity ("v", cl::desc("Set verbosity level"), cl::value_desc("verbosity"), cl::cat(LissCategory), cl::location(verbosity));
string lock_desc = "Set the maximum number of locks to synthesise (default=" + to_string(lock_limit) + ")";
cl::opt<unsigned, true> Lock_Limit ("locklimit", cl::desc(lock_desc.c_str()), cl::value_desc("max locks"), cl::cat(LissCategory), cl::location(lock_limit));
cl::opt<bool, true> Synthesis_Print_SMT_Only ("print-smt-only", cl::desc("Only dump the SMT formula representing all valid locks"), cl::value_desc("print valid lock SMT only"), cl::cat(LissCategory), cl::location(print_smt_only));

string bound_desc = "Set bound of antichain algorithm (default=" + to_string(max_bound) + ")";
cl::opt<unsigned, true> Bound ("bound", cl::desc(bound_desc.c_str()), cl::value_desc("bound"), cl::cat(LissCategory), cl::location(max_bound));

std::vector<actions::actionp> acts;

class ActionRunnerAction : public clang::ASTFrontendAction {
public:
  virtual std::unique_ptr<clang::ASTConsumer> CreateASTConsumer(
    clang::CompilerInstance &Compiler, llvm::StringRef InFile) {
    return std::unique_ptr<clang::ASTConsumer>(
      new clang_interf::abstraction_consumer(Compiler, acts));
    }
};



int main(int argc, const char **argv) {
  for (int i = 0; i < argc; ++i) {
    if (strncmp("-version", argv[i], 8)==0) {
      cout << "Liss:" << endl;
      cout << "  GIT SHA1:    " << GIT_SHA1 << endl;
      cout << "  GIT REFSPEC: " << GIT_REFSPEC << endl;
    }
  }
  CommonOptionsParser OptionsParser(argc, argv, LissCategory);
  
  for (actions::action_names a : ActionList) {
    acts.push_back(actions::create_action(a));
  }
  
  if (OptionsParser.getSourcePathList().empty()) {
    cerr << "At least one input file needs to be specified" << endl;
    return 1;
  }
  string file = OptionsParser.getSourcePathList().front();
  boost::filesystem::path file_path(file);
  main_file_path = file_path.string();
  file_path = boost::filesystem::absolute(file_path);
  if (!boost::filesystem::exists(file_path)) {
    cerr << "File " << file_path << " not found!" << endl;
    return 1;
  }
  main_filename = file_path.filename().string();
  start_file_code = file_path.string();
  start_file_code.replace(start_file_code.length()-2,2, ".start.c");
  output_file_log = file_path.string();
  output_file_log.replace(output_file_log.length()-2,2, ".trace.txt");
  boost::filesystem::path folder_path = file_path.replace_extension("output");
  if (boost::filesystem::exists(folder_path)) {
    for (boost::filesystem::directory_iterator end_dir_it, it(folder_path); it!=end_dir_it; ++it) {
      boost::filesystem::remove_all(it->path());
    }
  } else {
    // delay creation of the debug folder
  }
  debug_folder = folder_path.string() + "/";

  
  
  ClangTool Tool(OptionsParser.getCompilations(),
                 OptionsParser.getSourcePathList());
  int ret = Tool.run(newFrontendActionFactory<ActionRunnerAction>().get());
  return ret;
}
