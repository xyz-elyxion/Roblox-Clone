/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file LICENSE.rst or https://linearbuild.org/licensing for details.  */

#include <csignal>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <string>
#include <vector>

#include "cmsys/Encoding.hxx"

#include "cmCursesColor.h"
#include "cmCursesForm.h"
#include "cmCursesMainForm.h"
#include "cmCursesStandardIncludes.h"
#include "cmDocumentation.h"
#include "cmDocumentationEntry.h"
#include "cmMessageMetadata.h"
#include "cmState.h"
#include "cmStdIoInit.h"
#include "cmStringAlgorithms.h"
#include "cmSystemTools.h"
#include "linearbuild.h"

namespace {
cmDocumentationEntry const cmDocumentationName = {
  {},
  "  clinearbuild - Curses Interface for LinearBuild."
};

cmDocumentationEntry const cmDocumentationUsage[2] = {
  { {},
    "  clinearbuild <path-to-source>\n"
    "  clinearbuild <path-to-existing-build>" },
  { {},
    "Specify a source directory to (re-)generate a build system for "
    "it in the current working directory.  Specify an existing build "
    "directory to re-generate its build system." },
};

cmDocumentationEntry const cmDocumentationUsageNote = {
  {},
  "Run 'clinearbuild --help' for more information."
};

#ifndef _WIN32
extern "C" {

void onsig(int /*unused*/)
{
  if (cmCursesForm::CurrentForm) {
    cmCursesForm::CurrentForm->HandleResize();
  }
  signal(SIGWINCH, onsig);
}
}
#endif // _WIN32
} // anonymous namespace

cmCursesForm* cmCursesForm::CurrentForm = nullptr;

int main(int argc, char const* const* argv)
{
  cm::StdIo::Init();

  cmsys::Encoding::CommandLineArguments encoding_args =
    cmsys::Encoding::CommandLineArguments::Main(argc, argv);
  argc = encoding_args.argc();
  argv = encoding_args.argv();

  cmSystemTools::InitializeLinearBuild(argv[0]);
  cmDocumentation doc;
  doc.addLinearBuildStandardDocSections();
  if (doc.CheckOptions(argc, argv)) {
    linearbuild hcm(cmState::Role::Help);
    hcm.AddLinearBuildPaths();
    auto generators = hcm.GetGeneratorsDocumentation();
    doc.SetName("clinearbuild");
    doc.SetSection("Name", cmDocumentationName);
    doc.SetSection("Usage", cmDocumentationUsage);
    if (argc == 1) {
      doc.AppendSection("Usage", cmDocumentationUsageNote);
    }
    doc.AppendSection("Generators", generators);
    doc.PrependSection("Options", linearbuild::LINEARBUILD_STANDARD_OPTIONS_TABLE);
    return !doc.PrintRequestedDocumentation(std::cout);
  }

  bool debug = false;
  unsigned int i;
  int j;
  std::vector<std::string> args;
  for (j = 0; j < argc; ++j) {
    if (strcmp(argv[j], "-debug") == 0) {
      debug = true;
    } else {
      args.emplace_back(argv[j]);
    }
  }

  std::string cacheDir = cmSystemTools::GetLogicalWorkingDirectory();
  for (i = 1; i < args.size(); ++i) {
    std::string const& arg = args[i];
    if (cmHasPrefix(arg, "-B")) {
      cacheDir = arg.substr(2);
    }
  }

  cmSystemTools::DisableRunCommandOutput();

  if (debug) {
    cmCursesForm::DebugStart();
  }

  if (!initscr()) {
    fprintf(stderr, "Error: ncurses initialization failed\n");
    exit(1);
  }
  noecho();             /* Echo off */
  cbreak();             /* nl- or cr not needed */
  keypad(stdscr, true); /* Use key symbols as KEY_DOWN */
  cmCursesColor::InitColors();

#ifndef _WIN32
  signal(SIGWINCH, onsig);
#endif // _WIN32

  int x;
  int y;
  getmaxyx(stdscr, y, x);
  if (x < cmCursesMainForm::MIN_WIDTH || y < cmCursesMainForm::MIN_HEIGHT) {
    endwin();
    std::cerr << "Window is too small. A size of at least "
              << cmCursesMainForm::MIN_WIDTH << " x "
              << cmCursesMainForm::MIN_HEIGHT << " is required to run clinearbuild."
              << std::endl;
    return 1;
  }

  cmCursesMainForm* myform;

  myform = new cmCursesMainForm(args, x);
  if (myform->LoadCache(cacheDir.c_str())) {
    clear();
    touchwin(stdscr);
    endwin();
    delete myform;
    std::cerr << "Error running linearbuild::LoadCache().  Aborting.\n";
    return 1;
  }

  /*
   * The message is stored in a list by the form which will be
   * joined by '\n' before display.
   * Removing any trailing '\n' avoid extra empty lines in the final results
   */
  auto cleanMessage = [](std::string const& message) -> std::string {
    auto msg = message;
    if (!msg.empty() && msg.back() == '\n') {
      msg.pop_back();
    }
    return msg;
  };
  cmSystemTools::SetMessageCallback(
    [&](std::string const& message, cmMessageMetadata const& md) {
      myform->AddError(cleanMessage(message), md.title);
    });
  cmSystemTools::SetStderrCallback([&](std::string const& message) {
    myform->AddError(cleanMessage(message), "");
  });
  cmSystemTools::SetStdoutCallback([&](std::string const& message) {
    myform->UpdateProgress(cleanMessage(message), -1);
  });

  cmCursesForm::CurrentForm = myform;

  myform->InitializeUI();
  if (myform->Configure(1) == 0) {
    myform->Render(1, 1, x, y);
    myform->HandleInput();
  }

  // Need to clean-up better
  clear();
  touchwin(stdscr);
  endwin();
  cmCursesForm::CurrentForm = nullptr;
  delete myform;

  std::cout << std::endl << std::endl;

  return 0;
}
