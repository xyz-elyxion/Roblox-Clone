/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file LICENSE.rst or https://rblxbuild.org/licensing for details.  */
#include "RBLXBuildSetupDialog.h"

#include <cm/memory>

#include <QCloseEvent>
#include <QCoreApplication>
#include <QDesktopServices>
#include <QDialogButtonBox>
#include <QDragEnterEvent>
#include <QFileDialog>
#include <QInputDialog>
#include <QKeySequence>
#include <QMenu>
#include <QMenuBar>
#include <QMessageBox>
#include <QMimeData>
#include <QProcessEnvironment>
#include <QProgressBar>
#include <QSettings>
#include <QShortcut>
#include <QStatusBar>
#include <QString>
#include <QUrl>
#include <QVector>

#ifdef QT_WINEXTRAS
#  include <QWinTaskbarButton>
#  include <QWinTaskbarProgress>
#endif

#include "cmSystemTools.h"
#include "cmVersion.h"

#include "AddCacheEntry.h"
#include "EnvironmentDialog.h"
#include "FirstConfigure.h"
#include "QRBLXBuild.h"
#include "QRBLXBuildCacheView.h"
#include "QRBLXBuildSizeType.h"
#include "RegexExplorer.h"
#include "WarningMessagesDialog.h"

void OpenReferenceManual(QString const& filename)
{
  QString urlFormat("https://rblxbuild.org/rblxbuild/help/v%1.%2/");
  QUrl url(urlFormat.arg(QString::number(cmVersion::GetMajorVersion()),
                         QString::number(cmVersion::GetMinorVersion())));

  if (!cmSystemTools::GetHTMLDoc().empty()) {
    url = QUrl::fromLocalFile(
      QDir(QString::fromStdString(cmSystemTools::GetHTMLDoc()))
        .filePath(filename));
  }

  QDesktopServices::openUrl(url);
}

namespace {
QString const PRESETS_DISABLED_TOOLTIP =
  "This option is disabled because there are no available presets in "
  "RBLXBuildPresets.json or RBLXBuildUserPresets.json.";
}

QRBLXBuildThread::QRBLXBuildThread(QObject* p)
  : QThread(p)
{
}

QRBLXBuild* QRBLXBuildThread::rblxbuildInstance() const
{
  return this->RBLXBuildInstance.get();
}

void QRBLXBuildThread::run()
{
  this->RBLXBuildInstance = cm::make_unique<QRBLXBuild>();
  // emit that this rblxbuild thread is ready for use
  emit this->rblxbuildInitialized();
  this->exec();
  this->RBLXBuildInstance.reset();
}

RBLXBuildSetupDialog::RBLXBuildSetupDialog()
  : ExitAfterGenerate(true)
  , CacheModified(false)
  , ConfigureNeeded(true)
  , CurrentState(Interrupting)
{
  QString title = QString(tr("RBLXBuild %1"));
  title = title.arg(cmVersion::GetRBLXBuildVersion());
  this->setWindowTitle(title);

  // create the GUI
  QSettings settings;
  settings.beginGroup("Settings/StartPath");
  restoreGeometry(settings.value("geometry").toByteArray());
  restoreState(settings.value("windowState").toByteArray());

  this->AddVariableNames =
    settings.value("AddVariableNames", QStringList("RBLXBUILD_INSTALL_PREFIX"))
      .toStringList();
  this->AddVariableTypes =
    settings.value("AddVariableTypes", QStringList("PATH")).toStringList();

  QWidget* cont = new QWidget(this);
  this->setupUi(cont);
  this->Splitter->setStretchFactor(0, 3);
  this->Splitter->setStretchFactor(1, 1);
  this->setCentralWidget(cont);
  this->ProgressBar->reset();
  this->RemoveEntry->setEnabled(false);
  this->AddEntry->setEnabled(false);
  this->Preset->setStatusTip(PRESETS_DISABLED_TOOLTIP);

  QByteArray p = settings.value("SplitterSizes").toByteArray();
  this->Splitter->restoreState(p);

  bool groupView = settings.value("GroupView", false).toBool();
  this->setGroupedView(groupView);
  this->groupedCheck->setCheckState(groupView ? Qt::Checked : Qt::Unchecked);

  bool advancedView = settings.value("AdvancedView", false).toBool();
  this->setAdvancedView(advancedView);
  this->advancedCheck->setCheckState(advancedView ? Qt::Checked
                                                  : Qt::Unchecked);

  QMenu* FileMenu = this->menuBar()->addMenu(tr("&File"));
  this->ReloadCacheAction = FileMenu->addAction(tr("&Reload Cache"));
  QObject::connect(this->ReloadCacheAction, &QAction::triggered, this,
                   &RBLXBuildSetupDialog::doReloadCache);
  this->DeleteCacheAction = FileMenu->addAction(tr("&Delete Cache"));
  QObject::connect(this->DeleteCacheAction, &QAction::triggered, this,
                   &RBLXBuildSetupDialog::doDeleteCache);
  this->ExitAction = FileMenu->addAction(tr("E&xit"));
  QObject::connect(this->ExitAction, &QAction::triggered, this,
                   &RBLXBuildSetupDialog::close);
  this->ExitAction->setShortcut(QKeySequence::Quit);

  QMenu* ToolsMenu = this->menuBar()->addMenu(tr("&Tools"));
  this->ConfigureAction = ToolsMenu->addAction(tr("&Configure"));
  QObject::connect(this->ConfigureAction, &QAction::triggered, this,
                   &RBLXBuildSetupDialog::doConfigure);
  // prevent merging with Preferences menu item on macOS
  this->ConfigureAction->setMenuRole(QAction::NoRole);
  this->GenerateAction = ToolsMenu->addAction(tr("&Generate"));
  QObject::connect(this->GenerateAction, &QAction::triggered, this,
                   &RBLXBuildSetupDialog::doGenerate);
  auto* a = ToolsMenu->addAction(tr("&Show My Changes"));
  QObject::connect(a, &QAction::triggered, this,
                   &RBLXBuildSetupDialog::showUserChanges);
#if defined(Q_WS_MAC) || defined(Q_OS_MAC)
  this->InstallForCommandLineAction =
    ToolsMenu->addAction(tr("&How to Install For Command Line Use"));
  QObject::connect(this->InstallForCommandLineAction, &QAction::triggered,
                   this, &RBLXBuildSetupDialog::doInstallForCommandLine);
#endif
  ToolsMenu->addSeparator();
  a = ToolsMenu->addAction(tr("Regular Expression Explorer..."));
  QObject::connect(a, &QAction::triggered, this,
                   &RBLXBuildSetupDialog::doRegexExplorerDialog);
  ToolsMenu->addSeparator();
  a = ToolsMenu->addAction(tr("&Find in Output..."));
  QObject::connect(a, &QAction::triggered, this,
                   &RBLXBuildSetupDialog::doOutputFindDialog);
  a->setShortcut(QKeySequence::Find);
  a = ToolsMenu->addAction(tr("Find Next"));
  QObject::connect(a, &QAction::triggered, this,
                   &RBLXBuildSetupDialog::doOutputFindNext);
  a->setShortcut(QKeySequence::FindNext);
  a = ToolsMenu->addAction(tr("Find Previous"));
  QObject::connect(a, &QAction::triggered, this,
                   &RBLXBuildSetupDialog::doOutputFindPrev);
  a->setShortcut(QKeySequence::FindPrevious);
  a = ToolsMenu->addAction(tr("Goto Next Error")); // in Visual Studio
  QObject::connect(a, &QAction::triggered, this,
                   &RBLXBuildSetupDialog::doOutputErrorNext);
  a->setShortcut(QKeySequence(Qt::Key_F8));
  auto* s = new QShortcut(this);
#if (QT_VERSION < QT_VERSION_CHECK(6, 0, 0))
  s->setKey(QKeySequence(Qt::CTRL + Qt::Key_Period));
#else
  s->setKey(QKeySequence(Qt::CTRL | Qt::Key_Period));
#endif
  QObject::connect(s, &QShortcut::activated, this,
                   &RBLXBuildSetupDialog::doOutputErrorNext); // in Eclipse

  QMenu* OptionsMenu = this->menuBar()->addMenu(tr("&Options"));
  a = OptionsMenu->addAction(tr("Warning Messages..."));
  QObject::connect(a, &QAction::triggered, this,
                   &RBLXBuildSetupDialog::doWarningMessagesDialog);

  QAction* debugAction = OptionsMenu->addAction(tr("&Debug Output"));
  debugAction->setCheckable(true);
  QObject::connect(debugAction, &QAction::toggled, this,
                   &RBLXBuildSetupDialog::setDebugOutput);

  OptionsMenu->addSeparator();
  a = OptionsMenu->addAction(tr("&Expand Grouped Entries"));
  QObject::connect(a, &QAction::triggered, this->CacheValues,
                   &QRBLXBuildCacheView::expandAll);
  a = OptionsMenu->addAction(tr("&Collapse Grouped Entries"));
  QObject::connect(a, &QAction::triggered, this->CacheValues,
                   &QRBLXBuildCacheView::collapseAll);

  QMenu* HelpMenu = this->menuBar()->addMenu(tr("&Help"));
  a = HelpMenu->addAction(tr("Help"));
  QObject::connect(a, &QAction::triggered, this, &RBLXBuildSetupDialog::doHelp);
  a->setShortcut(QKeySequence::HelpContents);
  a = HelpMenu->addAction(tr("RBLXBuild Reference Manual"));
  QObject::connect(a, &QAction::triggered, this,
                   [] { OpenReferenceManual("index.html"); });
  a = HelpMenu->addAction(tr("About"));
  QObject::connect(a, &QAction::triggered, this, &RBLXBuildSetupDialog::doAbout);

  this->setAcceptDrops(true);

  // get the saved binary directories
  QStringList buildPaths = this->loadBuildPaths();
  this->BinaryDirectory->addItems(buildPaths);

  this->BinaryDirectory->setCompleter(new QRBLXBuildFileCompleter(this, true));
  this->SourceDirectory->setCompleter(new QRBLXBuildFileCompleter(this, true));

  // fixed pitch font in output window
  QFont outputFont("Courier New");
  outputFont.setStyleHint(QFont::Monospace);
  this->Output->setFont(outputFont);
  this->ErrorFormat.setForeground(QBrush(Qt::red));

  this->Output->setContextMenuPolicy(Qt::CustomContextMenu);
  connect(this->Output, &QTextEdit::customContextMenuRequested, this,
          &RBLXBuildSetupDialog::doOutputContextMenu);

  // disable open project button
  this->OpenProjectButton->setDisabled(true);

  // start the rblxbuild worker thread
  this->RBLXBuildThread = new QRBLXBuildThread(this);
  QObject::connect(this->RBLXBuildThread, &QRBLXBuildThread::rblxbuildInitialized, this,
                   &RBLXBuildSetupDialog::initialize, Qt::QueuedConnection);
  this->RBLXBuildThread->start();

  this->enterState(ReadyConfigure);

  ProgressOffset = 0.0;
  ProgressFactor = 1.0;
}

void RBLXBuildSetupDialog::initialize()
{
  // now the rblxbuild worker thread is running, lets make our connections to it
  QObject::connect(this->RBLXBuildThread->rblxbuildInstance(),
                   &QRBLXBuild::propertiesChanged, this->CacheValues->cacheModel(),
                   &QRBLXBuildCacheModel::setProperties);

  QObject::connect(this->ConfigureButton, &QAbstractButton::clicked, this,
                   &RBLXBuildSetupDialog::doConfigure);

  QObject::connect(this->RBLXBuildThread->rblxbuildInstance(), &QRBLXBuild::configureDone,
                   this, &RBLXBuildSetupDialog::exitLoop);
  QObject::connect(this->RBLXBuildThread->rblxbuildInstance(), &QRBLXBuild::generateDone,
                   this, &RBLXBuildSetupDialog::exitLoop);

  QObject::connect(this->GenerateButton, &QAbstractButton::clicked, this,
                   &RBLXBuildSetupDialog::doGenerate);
  QObject::connect(this->OpenProjectButton, &QAbstractButton::clicked, this,
                   &RBLXBuildSetupDialog::doOpenProject);

  QObject::connect(this->BrowseSourceDirectoryButton,
                   &QAbstractButton::clicked, this,
                   &RBLXBuildSetupDialog::doSourceBrowse);
  QObject::connect(this->ReloadPresetsButton, &QAbstractButton::clicked,
                   this->RBLXBuildThread->rblxbuildInstance(), &QRBLXBuild::loadPresets);
  QObject::connect(this->BrowseBinaryDirectoryButton,
                   &QAbstractButton::clicked, this,
                   &RBLXBuildSetupDialog::doBinaryBrowse);

  QObject::connect(this->BinaryDirectory, &QComboBox::editTextChanged, this,
                   &RBLXBuildSetupDialog::onBinaryDirectoryChanged);
  QObject::connect(this->SourceDirectory, &QLineEdit::textChanged, this,
                   &RBLXBuildSetupDialog::onSourceDirectoryChanged);
  QObject::connect(this->Preset, &QRBLXBuildPresetComboBox::presetChanged, this,
                   &RBLXBuildSetupDialog::onBuildPresetChanged);

  QObject::connect(this->RBLXBuildThread->rblxbuildInstance(),
                   &QRBLXBuild::sourceDirChanged, this,
                   &RBLXBuildSetupDialog::updateSourceDirectory);
  QObject::connect(this->RBLXBuildThread->rblxbuildInstance(),
                   &QRBLXBuild::binaryDirChanged, this,
                   &RBLXBuildSetupDialog::updateBinaryDirectory);
  QObject::connect(this->RBLXBuildThread->rblxbuildInstance(), &QRBLXBuild::presetsChanged,
                   this, &RBLXBuildSetupDialog::updatePresets);
  QObject::connect(this->RBLXBuildThread->rblxbuildInstance(), &QRBLXBuild::presetChanged,
                   this, &RBLXBuildSetupDialog::updatePreset);
  QObject::connect(this->RBLXBuildThread->rblxbuildInstance(), &QRBLXBuild::presetApplied,
                   this, &RBLXBuildSetupDialog::onPresetApplied);
  QObject::connect(this->RBLXBuildThread->rblxbuildInstance(),
                   &QRBLXBuild::presetLoadError, this,
                   &RBLXBuildSetupDialog::showPresetLoadError);

  QObject::connect(this->RBLXBuildThread->rblxbuildInstance(),
                   &QRBLXBuild::progressChanged, this,
                   &RBLXBuildSetupDialog::showProgress);

  QObject::connect(this->RBLXBuildThread->rblxbuildInstance(), &QRBLXBuild::errorMessage,
                   this, &RBLXBuildSetupDialog::error);

  QObject::connect(this->RBLXBuildThread->rblxbuildInstance(), &QRBLXBuild::outputMessage,
                   this, &RBLXBuildSetupDialog::message);

  QObject::connect(this->RBLXBuildThread->rblxbuildInstance(), &QRBLXBuild::openPossible,
                   this->OpenProjectButton, &RBLXBuildSetupDialog::setEnabled);

  QObject::connect(this->groupedCheck, &QCheckBox::toggled, this,
                   &RBLXBuildSetupDialog::setGroupedView);
  QObject::connect(this->advancedCheck, &QCheckBox::toggled, this,
                   &RBLXBuildSetupDialog::setAdvancedView);
  QObject::connect(this->Search, &QLineEdit::textChanged, this,
                   &RBLXBuildSetupDialog::setSearchFilter);

  QObject::connect(this->RBLXBuildThread->rblxbuildInstance(),
                   &QRBLXBuild::generatorChanged, this,
                   &RBLXBuildSetupDialog::updateGeneratorLabel);
  this->updateGeneratorLabel(QString());

  QObject::connect(this->CacheValues->cacheModel(),
                   &QRBLXBuildCacheModel::dataChanged, this,
                   &RBLXBuildSetupDialog::setCacheModified);

  QObject::connect(this->CacheValues->selectionModel(),
                   &QItemSelectionModel::selectionChanged, this,
                   &RBLXBuildSetupDialog::selectionChanged);
  QObject::connect(this->RemoveEntry, &QAbstractButton::clicked, this,
                   &RBLXBuildSetupDialog::removeSelectedCacheEntries);
  QObject::connect(this->AddEntry, &QAbstractButton::clicked, this,
                   &RBLXBuildSetupDialog::addCacheEntry);

  QObject::connect(this->Environment, &QAbstractButton::clicked, this,
                   &RBLXBuildSetupDialog::editEnvironment);

  if (!this->SourceDirectory->text().isEmpty() &&
      !this->DeferredPreset.isNull()) {
    this->onSourceDirectoryChanged(this->SourceDirectory->text());
    if (!this->BinaryDirectory->lineEdit()->text().isEmpty()) {
      this->onBinaryDirectoryChanged(
        this->BinaryDirectory->lineEdit()->text());
    }
  } else if (!this->SourceDirectory->text().isEmpty() ||
             !this->BinaryDirectory->lineEdit()->text().isEmpty()) {
    if (this->SourceDirectory->text().isEmpty() &&
        !cmSystemTools::FileIsFullPath(
          this->BinaryDirectory->lineEdit()->text().toStdString())) {
      // If the binary directory is relative, load the previous source path
      // from the config
      QSettings settings;
      settings.beginGroup("Settings/StartPath");
      QString srcDir = settings.value(QString("WhereSource")).toString();
      this->SourceDirectory->blockSignals(true);
      this->SourceDirectory->setText(srcDir);
      this->SourceDirectory->blockSignals(false);
    }
    this->onSourceDirectoryChanged(this->SourceDirectory->text());
    this->onBinaryDirectoryChanged(this->BinaryDirectory->lineEdit()->text());
  } else {
    this->onBinaryDirectoryChanged(this->BinaryDirectory->lineEdit()->text());
  }

#ifdef QT_WINEXTRAS
  this->TaskbarButton = new QWinTaskbarButton(this);
  this->TaskbarButton->setWindow(this->windowHandle());
#endif
}

RBLXBuildSetupDialog::~RBLXBuildSetupDialog()
{
  QSettings settings;
  settings.beginGroup("Settings/StartPath");
  settings.setValue("windowState", QVariant(saveState()));
  settings.setValue("geometry", QVariant(saveGeometry()));
  settings.setValue("SplitterSizes", this->Splitter->saveState());

  // wait for thread to stop
  this->RBLXBuildThread->quit();
  this->RBLXBuildThread->wait();
}

bool RBLXBuildSetupDialog::prepareConfigure()
{
  // make sure build directory exists
  QString bindir = this->RBLXBuildThread->rblxbuildInstance()->binaryDirectory();
  QDir dir(bindir);
  if (!dir.exists()) {
    QString msg = tr("Build directory does not exist, "
                     "should I create it?\n\n"
                     "Directory: ");
    msg += bindir;
    QString title = tr("Create Directory");
    QMessageBox::StandardButton btn;
    btn = QMessageBox::information(this, title, msg,
                                   QMessageBox::Yes | QMessageBox::No);
    if (btn == QMessageBox::No) {
      return false;
    }
    if (!dir.mkpath(".")) {
      QMessageBox::information(
        this, tr("Create Directory Failed"),
        QString(tr("Failed to create directory %1")).arg(dir.path()),
        QMessageBox::Ok);

      return false;
    }
  }

  // if no generator, prompt for it and other setup stuff
  if (this->RBLXBuildThread->rblxbuildInstance()->generator().isEmpty()) {
    if (!this->setupFirstConfigure()) {
      return false;
    }
  }

  // remember paths
  this->addBinaryPath(
    this->RBLXBuildThread->rblxbuildInstance()->relativeBinaryDirectory());
  QSettings settings;
  settings.beginGroup("Settings/StartPath");
  settings.setValue("WhereSource",
                    this->RBLXBuildThread->rblxbuildInstance()->sourceDirectory());

  return true;
}

void RBLXBuildSetupDialog::exitLoop(int err)
{
  this->LocalLoop.exit(err);
}

void RBLXBuildSetupDialog::doConfigure()
{
  if (this->CurrentState == Configuring) {
    // stop configure
    doInterrupt();
    return;
  }

  // Preset still applying: its cache values aren't in the model yet.
  if (this->PresetApplicationPending) {
    return;
  }

  if (!prepareConfigure()) {
    return;
  }

  this->enterState(Configuring);

  bool ret = doConfigureInternal();

  if (ret) {
    this->ConfigureNeeded = false;
  }

  if (ret && !this->CacheValues->cacheModel()->newPropertyCount()) {
    this->enterState(ReadyGenerate);
  } else {
    this->enterState(ReadyConfigure);
    this->CacheValues->scrollToTop();
  }
  this->ProgressBar->reset();

#ifdef QT_WINEXTRAS
  this->TaskbarButton->progress()->reset();
#endif
}

bool RBLXBuildSetupDialog::doConfigureInternal()
{
  this->Output->clear();
  this->CacheValues->selectionModel()->clear();

  QMetaObject::invokeMethod(
    this->RBLXBuildThread->rblxbuildInstance(), "setProperties", Qt::QueuedConnection,
    Q_ARG(QRBLXBuildPropertyList, this->CacheValues->cacheModel()->properties()));
  QMetaObject::invokeMethod(this->RBLXBuildThread->rblxbuildInstance(), "configure",
                            Qt::QueuedConnection);

  int err = this->LocalLoop.exec();

  if (err != 0) {
    QMessageBox::critical(
      this, tr("Error"),
      tr("Error in configuration process, project files may be invalid"),
      QMessageBox::Ok);
  }

  return 0 == err;
}

void RBLXBuildSetupDialog::doInstallForCommandLine()
{
  QString title = tr("How to Install For Command Line Use");
  QString msg = tr("One may add RBLXBuild to the PATH:\n"
                   "\n"
                   " PATH=\"%1\":\"$PATH\"\n"
                   "\n"
                   "Or, to install symlinks to '/usr/local/bin', run:\n"
                   "\n"
                   " sudo \"%2\" --install\n"
                   "\n"
                   "Or, to install symlinks to another directory, run:\n"
                   "\n"
                   " sudo \"%3\" --install=/path/to/bin\n");
  msg = msg.arg(
    cmSystemTools::GetFilenamePath(cmSystemTools::GetRBLXBuildCommand()).c_str());
  msg = msg.arg(cmSystemTools::GetRBLXBuildGUICommand().c_str());
  msg = msg.arg(cmSystemTools::GetRBLXBuildGUICommand().c_str());

  QDialog dialog;
  dialog.setWindowTitle(title);
  QVBoxLayout* l = new QVBoxLayout(&dialog);
  QLabel* lab = new QLabel(&dialog);
  l->addWidget(lab);
  lab->setText(msg);
  lab->setWordWrap(false);
  lab->setTextInteractionFlags(Qt::TextSelectableByMouse);
  QDialogButtonBox* btns =
    new QDialogButtonBox(QDialogButtonBox::Ok, Qt::Horizontal, &dialog);
  QObject::connect(btns, &QDialogButtonBox::accepted, &dialog,
                   &QDialog::accept);
  l->addWidget(btns);
  dialog.exec();
}

bool RBLXBuildSetupDialog::doGenerateInternal()
{
  QMetaObject::invokeMethod(this->RBLXBuildThread->rblxbuildInstance(), "generate",
                            Qt::QueuedConnection);

  int err = this->LocalLoop.exec();

  if (err != 0) {
    QMessageBox::critical(
      this, tr("Error"),
      tr("Error in generation process, project files may be invalid"),
      QMessageBox::Ok);
  }

  return 0 == err;
}

void RBLXBuildSetupDialog::doGenerate()
{
  if (this->CurrentState == Generating) {
    // stop generate
    doInterrupt();
    return;
  }

  // Preset still applying: prepareConfigure() would read worker state that
  // applyPreset() is still mutating.
  if (this->PresetApplicationPending) {
    return;
  }

  // see if we need to configure
  // we'll need to configure if:
  //   the configure step hasn't been done yet
  //   generate was the last step done
  if (this->ConfigureNeeded) {
    if (!prepareConfigure()) {
      return;
    }
  }

  this->enterState(Generating);

  bool config_passed = true;
  if (this->ConfigureNeeded) {
    this->CacheValues->cacheModel()->setShowNewProperties(false);
    this->ProgressFactor = 0.5;
    config_passed = doConfigureInternal();
    this->ProgressOffset = 0.5;
  }

  if (config_passed) {
    doGenerateInternal();
  }

  this->ProgressOffset = 0.0;
  this->ProgressFactor = 1.0;
  this->CacheValues->cacheModel()->setShowNewProperties(true);

  this->enterState(ReadyConfigure);
  this->ProgressBar->reset();
#ifdef QT_WINEXTRAS
  this->TaskbarButton->progress()->reset();
#endif

  this->ConfigureNeeded = true;
}

void RBLXBuildSetupDialog::doOpenProject()
{
  QMetaObject::invokeMethod(this->RBLXBuildThread->rblxbuildInstance(), "open",
                            Qt::QueuedConnection);
}

void RBLXBuildSetupDialog::closeEvent(QCloseEvent* e)
{
  // prompt for close if there are unsaved changes, and we're not busy
  if (this->CacheModified) {
    QString msg = tr("You have changed options but not rebuilt, "
                     "are you sure you want to exit?");
    QString title = tr("Confirm Exit");
    QMessageBox::StandardButton btn;
    btn = QMessageBox::critical(this, title, msg,
                                QMessageBox::Yes | QMessageBox::No);
    if (btn == QMessageBox::No) {
      e->ignore();
    }
  }

  // don't close if we're busy, unless the user really wants to
  if (this->CurrentState == Configuring) {
    QString msg =
      tr("You are in the middle of a Configure.\n"
         "If you Exit now the configure information will be lost.\n"
         "Are you sure you want to Exit?");
    QString title = tr("Confirm Exit");
    QMessageBox::StandardButton btn;
    btn = QMessageBox::critical(this, title, msg,
                                QMessageBox::Yes | QMessageBox::No);
    if (btn == QMessageBox::No) {
      e->ignore();
    } else {
      this->doInterrupt();
    }
  }

  // let the generate finish
  if (this->CurrentState == Generating) {
    e->ignore();
  }
}

void RBLXBuildSetupDialog::doHelp()
{
  QString msg = tr(
    "RBLXBuild is used to configure and generate build files for "
    "software projects.   The basic steps for configuring a project are as "
    "follows:\r\n\r\n1. Select the source directory for the project.  This "
    "should "
    "contain the RBLXBuildLists.txt files for the project.\r\n\r\n2. Select the "
    "build "
    "directory for the project.   This is the directory where the project "
    "will be "
    "built.  It can be the same or a different directory than the source "
    "directory.   For easy clean up, a separate build directory is "
    "recommended. "
    "RBLXBuild will create the directory if it does not exist.\r\n\r\n3. Once the "
    "source and binary directories are selected, it is time to press the "
    "Configure button.  This will cause RBLXBuild to read all of the input files "
    "and "
    "discover all the variables used by the project.   The first time a "
    "variable "
    "is displayed it will be in Red.   Users should inspect red variables "
    "making "
    "sure the values are correct.   For some projects the Configure process "
    "can "
    "be iterative, so continue to press the Configure button until there are "
    "no "
    "longer red entries.\r\n\r\n4. Once there are no longer red entries, you "
    "should click the Generate button.  This will write the build files to "
    "the build "
    "directory.");

  QDialog dialog;
  QFontMetrics met(this->font());
#if QT_VERSION >= QT_VERSION_CHECK(5, 11, 0)
  int msgWidth = met.horizontalAdvance(msg);
#else
  int msgWidth = met.width(msg);
#endif
  dialog.setMinimumSize(msgWidth / 15, 20);
  dialog.setWindowTitle(tr("Help"));
  QVBoxLayout* l = new QVBoxLayout(&dialog);
  QLabel* lab = new QLabel(&dialog);
  lab->setText(msg);
  lab->setWordWrap(true);
  QDialogButtonBox* btns =
    new QDialogButtonBox(QDialogButtonBox::Ok, Qt::Horizontal, &dialog);
  QObject::connect(btns, &QDialogButtonBox::accepted, &dialog,
                   &QDialog::accept);
  l->addWidget(lab);
  l->addWidget(btns);
  dialog.exec();
}

void RBLXBuildSetupDialog::doInterrupt()
{
  this->enterState(Interrupting);
  this->RBLXBuildThread->rblxbuildInstance()->interrupt();
}

void RBLXBuildSetupDialog::doSourceBrowse()
{
  QString dir = QFileDialog::getExistingDirectory(
    this, tr("Enter Path to Source"), this->SourceDirectory->text(),
    QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
  if (!dir.isEmpty()) {
    this->setSourceDirectory(dir);
  }
}

void RBLXBuildSetupDialog::updateSourceDirectory(QString const& dir)
{
  if (this->SourceDirectory->text() != dir) {
    this->SourceDirectory->blockSignals(true);
    this->SourceDirectory->setText(dir);
    this->SourceDirectory->blockSignals(false);
  }
}

void RBLXBuildSetupDialog::updateBinaryDirectory(QString const& dir)
{
  if (this->BinaryDirectory->currentText() != dir) {
    this->BinaryDirectory->blockSignals(true);
    this->BinaryDirectory->setEditText(dir);
    this->BinaryDirectory->blockSignals(false);
  }
}

void RBLXBuildSetupDialog::updatePresets(QVector<QRBLXBuildPreset> const& presets)
{
  if (this->Preset->presets() != presets) {
    this->Preset->blockSignals(true);
    this->Preset->setPresets(presets);
    this->Preset->blockSignals(false);
  }

  this->Preset->setDisabled(presets.isEmpty());
  this->Preset->setToolTip(presets.isEmpty() ? PRESETS_DISABLED_TOOLTIP : "");

  if (!this->DeferredPreset.isNull()) {
    QString const deferred = this->DeferredPreset;
    this->DeferredPreset = QString{};
    // An available preset selection submits a real request and keeps the gate
    // engaged; an unavailable one applies nothing, so release the gate here.
    this->Preset->setPresetName(deferred);
    if (this->Preset->presetName() != deferred) {
      this->PresetApplicationPending = false;
      this->updateCommandState();
    }
  }
}

void RBLXBuildSetupDialog::onPresetApplied(quint64 requestId, QString const& name,
                                       QRBLXBuildPropertyList const& properties)
{
  // Latest request wins; ignore a superseded completion.
  if (requestId != this->LatestPresetRequestId) {
    return;
  }

  this->CacheValues->cacheModel()->setProperties(properties);
  // Reconcile in case the combo drifted from what actually applied.
  if (this->Preset->presetName() != name) {
    this->Preset->blockSignals(true);
    this->Preset->setPresetName(name);
    this->Preset->blockSignals(false);
  }
  this->PresetApplicationPending = false;
  this->updateCommandState();
}

void RBLXBuildSetupDialog::updatePreset(QString const& name)
{
  if (this->Preset->presetName() != name) {
    this->Preset->blockSignals(true);
    this->Preset->setPresetName(name);
    this->Preset->blockSignals(false);
  }
}

void RBLXBuildSetupDialog::showPresetLoadError(QString const& dir,
                                           QString const& message)
{
  QMessageBox::warning(
    this, "Error Reading RBLXBuild Presets",
    QString("Could not read presets from %1: %2").arg(dir, message));
}

void RBLXBuildSetupDialog::doBinaryBrowse()
{
  QString abs_path = this->BinaryDirectory->currentText();
  if (!cmSystemTools::FileIsFullPath(abs_path.toStdString())) {
    if (!this->SourceDirectory->text().endsWith("/")) {
      abs_path = "/" + abs_path;
    }
    abs_path = this->SourceDirectory->text() + abs_path;
  }
  QString dir = QFileDialog::getExistingDirectory(
    this, tr("Enter Path to Build"), abs_path,
    QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
  if (!dir.isEmpty() && dir != this->BinaryDirectory->currentText()) {
    this->setBinaryDirectory(dir);
  }
}

void RBLXBuildSetupDialog::setBinaryDirectory(QString const& dir)
{
  this->BinaryDirectory->setEditText(dir);
}

void RBLXBuildSetupDialog::setStartupBinaryDirectory(bool startup)
{
  this->StartupBinaryDirectory = startup;
}

void RBLXBuildSetupDialog::onSourceDirectoryChanged(QString const& dir)
{
  this->Output->clear();
  QMetaObject::invokeMethod(this->RBLXBuildThread->rblxbuildInstance(),
                            "setSourceDirectory", Qt::QueuedConnection,
                            Q_ARG(QString, dir));
}

void RBLXBuildSetupDialog::onBinaryDirectoryChanged(QString const& dir)
{
  QString title = QString(tr("RBLXBuild %1 - %2"));
  title = title.arg(cmVersion::GetRBLXBuildVersion());
  title = title.arg(dir);
  this->setWindowTitle(title);

  this->CacheModified = false;
  this->CacheValues->cacheModel()->clear();
  qobject_cast<QRBLXBuildCacheModelDelegate*>(this->CacheValues->itemDelegate())
    ->clearChanges();
  this->Output->clear();
  QMetaObject::invokeMethod(this->RBLXBuildThread->rblxbuildInstance(),
                            "setBinaryDirectory", Qt::QueuedConnection,
                            Q_ARG(QString, dir));
}

void RBLXBuildSetupDialog::onBuildPresetChanged(QString const& name)
{
  // Applying is async: gate the dependent actions now, until the completion
  // lands.
  quint64 const requestId = ++this->LatestPresetRequestId;
  this->PresetApplicationPending = true;
  // First Generate after an apply must reconfigure with the new inputs.
  this->ConfigureNeeded = true;
  this->updateCommandState();

  QMetaObject::invokeMethod(this->RBLXBuildThread->rblxbuildInstance(), "applyPreset",
                            Qt::QueuedConnection, Q_ARG(QString, name),
                            Q_ARG(bool, !this->StartupBinaryDirectory),
                            Q_ARG(quint64, requestId));
  this->StartupBinaryDirectory = false;
}

void RBLXBuildSetupDialog::setSourceDirectory(QString const& dir)
{
  this->SourceDirectory->setText(dir);
}

void RBLXBuildSetupDialog::setDeferredPreset(QString const& preset)
{
  this->DeferredPreset = preset;
  // A --preset isn't applied until presets load; gate now so nothing runs
  // with default inputs during that startup window.
  if (!preset.isNull()) {
    this->PresetApplicationPending = true;
    this->updateCommandState();
  }
}

void RBLXBuildSetupDialog::showProgress(QString const& /*msg*/, float percent)
{
  percent = (percent * ProgressFactor) + ProgressOffset;
  this->ProgressBar->setValue(qRound(percent * 100));

#ifdef QT_WINEXTRAS
  QWinTaskbarProgress* progress = this->TaskbarButton->progress();
  progress->setVisible(true);
  progress->setValue(qRound(percent * 100));
#endif
}

void RBLXBuildSetupDialog::error(QString const& msg)
{
  this->Output->setCurrentCharFormat(this->ErrorFormat);
  // QTextEdit will terminate the msg with a ParagraphSeparator, but it also
  // replaces
  // all newlines with ParagraphSeparators. By replacing the newlines by
  // ourself, one
  // error msg will be one paragraph.
  QString paragraph(msg);
  paragraph.replace(QLatin1Char('\n'), QChar::LineSeparator);
  this->Output->append(paragraph);
}

void RBLXBuildSetupDialog::message(QString const& msg)
{
  this->Output->setCurrentCharFormat(this->MessageFormat);
  this->Output->append(msg);
}

void RBLXBuildSetupDialog::setEnabledState(bool enabled)
{
  // disable parts of the GUI during configure/generate
  this->CacheValues->cacheModel()->setEditEnabled(enabled);
  this->SourceDirectory->setEnabled(enabled);
  this->BrowseSourceDirectoryButton->setEnabled(enabled);
  this->Preset->setEnabled(enabled && !this->Preset->presets().isEmpty());
  this->BinaryDirectory->setEnabled(enabled);
  this->BrowseBinaryDirectoryButton->setEnabled(enabled);
  this->ReloadCacheAction->setEnabled(enabled);
  this->DeleteCacheAction->setEnabled(enabled);
  this->ExitAction->setEnabled(enabled);
  this->ConfigureAction->setEnabled(enabled);
  this->AddEntry->setEnabled(enabled);
  this->RemoveEntry->setEnabled(false); // let selection re-enable it
  this->Environment->setEnabled(enabled);
}

bool RBLXBuildSetupDialog::setupFirstConfigure()
{
  FirstConfigure dialog;

  // initialize dialog and restore saved settings

  // add generators
  dialog.setGenerators(
    this->RBLXBuildThread->rblxbuildInstance()->availableGenerators());

  // restore from settings
  dialog.loadFromSettings();

  auto presetData = this->Preset->currentData();
  if (presetData.isValid()) {
    auto preset = presetData.value<QRBLXBuildPreset>();
    dialog.setCurrentGenerator(preset.generator);
    if (preset.setArchitecture) {
      dialog.setPlatform(preset.architecture);
    }
    if (preset.setToolset) {
      dialog.setToolset(preset.toolset);
    }
    dialog.setCompilerOption(CompilerOption::DefaultPreset);
  }

  if (dialog.exec() == QDialog::Accepted) {
    dialog.saveToSettings();
    this->RBLXBuildThread->rblxbuildInstance()->setGenerator(dialog.getGenerator());
    this->RBLXBuildThread->rblxbuildInstance()->setPlatform(dialog.getPlatform());
    this->RBLXBuildThread->rblxbuildInstance()->setToolset(dialog.getToolset());

    QRBLXBuildCacheModel* m = this->CacheValues->cacheModel();

    if (dialog.compilerSetup()) {
      QString fortranCompiler = dialog.getFortranCompiler();
      if (!fortranCompiler.isEmpty()) {
        m->insertProperty(QRBLXBuildProperty::FILEPATH, "RBLXBUILD_Fortran_COMPILER",
                          "Fortran compiler.", fortranCompiler, false);
      }
      QString cxxCompiler = dialog.getCXXCompiler();
      if (!cxxCompiler.isEmpty()) {
        m->insertProperty(QRBLXBuildProperty::FILEPATH, "RBLXBUILD_CXX_COMPILER",
                          "CXX compiler.", cxxCompiler, false);
      }

      QString cCompiler = dialog.getCCompiler();
      if (!cCompiler.isEmpty()) {
        m->insertProperty(QRBLXBuildProperty::FILEPATH, "RBLXBUILD_C_COMPILER",
                          "C compiler.", cCompiler, false);
      }
    } else if (dialog.crossCompilerSetup()) {
      QString fortranCompiler = dialog.getFortranCompiler();
      if (!fortranCompiler.isEmpty()) {
        m->insertProperty(QRBLXBuildProperty::FILEPATH, "RBLXBUILD_Fortran_COMPILER",
                          "Fortran compiler.", fortranCompiler, false);
      }

      QString mode = dialog.getCrossIncludeMode();
      m->insertProperty(QRBLXBuildProperty::STRING,
                        "RBLXBUILD_FIND_ROOT_PATH_MODE_INCLUDE",
                        tr("RBLXBuild Find Include Mode"), mode, false);
      mode = dialog.getCrossLibraryMode();
      m->insertProperty(QRBLXBuildProperty::STRING,
                        "RBLXBUILD_FIND_ROOT_PATH_MODE_LIBRARY",
                        tr("RBLXBuild Find Library Mode"), mode, false);
      mode = dialog.getCrossProgramMode();
      m->insertProperty(QRBLXBuildProperty::STRING,
                        "RBLXBUILD_FIND_ROOT_PATH_MODE_PROGRAM",
                        tr("RBLXBuild Find Program Mode"), mode, false);

      QString rootPath = dialog.getCrossRoot();
      m->insertProperty(QRBLXBuildProperty::PATH, "RBLXBUILD_FIND_ROOT_PATH",
                        tr("RBLXBuild Find Root Path"), rootPath, false);

      QString systemName = dialog.getSystemName();
      m->insertProperty(QRBLXBuildProperty::STRING, "RBLXBUILD_SYSTEM_NAME",
                        tr("RBLXBuild System Name"), systemName, false);
      QString systemVersion = dialog.getSystemVersion();
      m->insertProperty(QRBLXBuildProperty::STRING, "RBLXBUILD_SYSTEM_VERSION",
                        tr("RBLXBuild System Version"), systemVersion, false);
      QString systemProcessor = dialog.getSystemProcessor();
      m->insertProperty(QRBLXBuildProperty::STRING, "RBLXBUILD_SYSTEM_PROCESSOR",
                        tr("RBLXBuild System Processor"), systemProcessor, false);
      QString cxxCompiler = dialog.getCXXCompiler();
      if (!cxxCompiler.isEmpty()) {
        m->insertProperty(QRBLXBuildProperty::FILEPATH, "RBLXBUILD_CXX_COMPILER",
                          tr("CXX compiler."), cxxCompiler, false);
      }
      QString cCompiler = dialog.getCCompiler();
      if (!cCompiler.isEmpty()) {
        m->insertProperty(QRBLXBuildProperty::FILEPATH, "RBLXBUILD_C_COMPILER",
                          tr("C compiler."), cCompiler, false);
      }
    } else if (dialog.crossCompilerToolChainFile()) {
      QString toolchainFile = dialog.getCrossCompilerToolChainFile();
      m->insertProperty(QRBLXBuildProperty::FILEPATH, "RBLXBUILD_TOOLCHAIN_FILE",
                        tr("Cross Compile ToolChain File"), toolchainFile,
                        false);
    }
    return true;
  }

  return false;
}

void RBLXBuildSetupDialog::updateGeneratorLabel(QString const& gen)
{
  QString str = tr("Current Generator: ");
  if (gen.isEmpty()) {
    str += tr("None");
  } else {
    str += gen;
  }
  this->Generator->setText(str);
}

void RBLXBuildSetupDialog::doReloadCache()
{
  QMetaObject::invokeMethod(this->RBLXBuildThread->rblxbuildInstance(), "reloadCache",
                            Qt::QueuedConnection);
}

void RBLXBuildSetupDialog::doDeleteCache()
{
  QString title = tr("Delete Cache");
  QString msg = tr("Are you sure you want to delete the cache?");
  QMessageBox::StandardButton btn;
  btn = QMessageBox::information(this, title, msg,
                                 QMessageBox::Yes | QMessageBox::No);
  if (btn == QMessageBox::No) {
    return;
  }
  QMetaObject::invokeMethod(this->RBLXBuildThread->rblxbuildInstance(), "deleteCache",
                            Qt::QueuedConnection);
}

void RBLXBuildSetupDialog::doAbout()
{
  QString msg = tr(
    "RBLXBuild %1 (rblxbuild.org).\n"
    "RBLXBuild suite maintained and supported by Kitware (kitware.com/rblxbuild).\n"
    "Distributed under terms of the BSD 3-Clause License.\n"
    "\n"
    "RBLXBuild GUI built using Qt %2 (qt-project.org).\n"
#ifdef USE_LGPL
    "\n"
    "The Qt Toolkit is Copyright (C) The Qt Company Ltd.\n"
    "Qt is licensed under terms of the GNU LGPLv" USE_LGPL ", available at:\n"
    " \"%3\""
#endif
  );
  msg = msg.arg(cmVersion::GetRBLXBuildVersion());
  msg = msg.arg(qVersion());
#ifdef USE_LGPL
  std::string lgpl =
    cmSystemTools::GetRBLXBuildRoot() + "/Licenses/LGPLv" USE_LGPL ".txt";
  msg = msg.arg(lgpl.c_str());
#endif

  QDialog dialog;
  dialog.setWindowTitle(tr("About"));
  QVBoxLayout* l = new QVBoxLayout(&dialog);
  QLabel* lab = new QLabel(&dialog);
  l->addWidget(lab);
  lab->setText(msg);
  lab->setWordWrap(true);
  QDialogButtonBox* btns =
    new QDialogButtonBox(QDialogButtonBox::Ok, Qt::Horizontal, &dialog);
  QObject::connect(btns, &QDialogButtonBox::accepted, &dialog,
                   &QDialog::accept);
  l->addWidget(btns);
  dialog.exec();
}

void RBLXBuildSetupDialog::setExitAfterGenerate(bool b)
{
  this->ExitAfterGenerate = b;
}

void RBLXBuildSetupDialog::addBinaryPath(QString const& path)
{
  QString cleanpath = QDir::cleanPath(path);

  // update UI
  this->BinaryDirectory->blockSignals(true);
  int idx = this->BinaryDirectory->findText(cleanpath);
  if (idx != -1) {
    this->BinaryDirectory->removeItem(idx);
  }
  this->BinaryDirectory->insertItem(0, cleanpath);
  this->BinaryDirectory->setCurrentIndex(0);
  this->BinaryDirectory->blockSignals(false);

  // save to registry
  QStringList buildPaths = this->loadBuildPaths();
  buildPaths.removeAll(cleanpath);
  buildPaths.prepend(cleanpath);
  this->saveBuildPaths(buildPaths);
}

void RBLXBuildSetupDialog::dragEnterEvent(QDragEnterEvent* e)
{
  if (!(this->CurrentState == ReadyConfigure ||
        this->CurrentState == ReadyGenerate)) {
    e->ignore();
    return;
  }

  QMimeData const* dat = e->mimeData();
  QList<QUrl> urls = dat->urls();
  QString file = urls.count() ? urls[0].toLocalFile() : QString();
  if (!file.isEmpty() &&
      (file.endsWith("RBLXBuildCache.txt", Qt::CaseInsensitive) ||
       file.endsWith("RBLXBuildLists.txt", Qt::CaseInsensitive))) {
    e->accept();
  } else {
    e->ignore();
  }
}

void RBLXBuildSetupDialog::dropEvent(QDropEvent* e)
{
  if (!(this->CurrentState == ReadyConfigure ||
        this->CurrentState == ReadyGenerate)) {
    return;
  }

  QMimeData const* dat = e->mimeData();
  QList<QUrl> urls = dat->urls();
  QString file = urls.count() ? urls[0].toLocalFile() : QString();
  if (file.endsWith("RBLXBuildCache.txt", Qt::CaseInsensitive)) {
    QFileInfo info(file);
    if (this->RBLXBuildThread->rblxbuildInstance()->binaryDirectory() !=
        info.absolutePath()) {
      this->setBinaryDirectory(info.absolutePath());
    }
  } else if (file.endsWith("RBLXBuildLists.txt", Qt::CaseInsensitive)) {
    QFileInfo info(file);
    if (this->RBLXBuildThread->rblxbuildInstance()->binaryDirectory() !=
        info.absolutePath()) {
      this->setSourceDirectory(info.absolutePath());
      this->setBinaryDirectory(info.absolutePath());
    }
  }
}

QStringList RBLXBuildSetupDialog::loadBuildPaths()
{
  QSettings settings;
  settings.beginGroup("Settings/StartPath");

  QStringList buildPaths;
  for (int i = 0; i < 10; i++) {
    QString p = settings.value(QString("WhereBuild%1").arg(i)).toString();
    if (!p.isEmpty()) {
      buildPaths.append(p);
    }
  }
  return buildPaths;
}

void RBLXBuildSetupDialog::saveBuildPaths(QStringList const& paths)
{
  QSettings settings;
  settings.beginGroup("Settings/StartPath");

  cm_qsizetype num = paths.count();
  if (num > 10) {
    num = 10;
  }

  for (cm_qsizetype i = 0; i < num; i++) {
    settings.setValue(QString("WhereBuild%1").arg(i), paths[i]);
  }
}

void RBLXBuildSetupDialog::setCacheModified()
{
  this->CacheModified = true;
  this->ConfigureNeeded = true;
  this->enterState(ReadyConfigure);
}

void RBLXBuildSetupDialog::removeSelectedCacheEntries()
{
  QModelIndexList idxs = this->CacheValues->selectionModel()->selectedRows();
  QList<QPersistentModelIndex> pidxs;
  foreach (QModelIndex const& i, idxs) {
    pidxs.append(i);
  }
  foreach (QPersistentModelIndex const& pi, pidxs) {
    this->CacheValues->model()->removeRow(pi.row(), pi.parent());
  }
}

void RBLXBuildSetupDialog::selectionChanged()
{
  QModelIndexList idxs = this->CacheValues->selectionModel()->selectedRows();
  if (idxs.count() &&
      (this->CurrentState == ReadyConfigure ||
       this->CurrentState == ReadyGenerate)) {
    this->RemoveEntry->setEnabled(true);
  } else {
    this->RemoveEntry->setEnabled(false);
  }
}

void RBLXBuildSetupDialog::enterState(RBLXBuildSetupDialog::State s)
{
  if (s == this->CurrentState) {
    return;
  }

  this->CurrentState = s;

  if (s == Interrupting) {
    this->ConfigureButton->setEnabled(false);
    this->GenerateButton->setEnabled(false);
    this->OpenProjectButton->setEnabled(false);
  } else if (s == Configuring) {
    this->setEnabledState(false);
    this->GenerateButton->setEnabled(false);
    this->GenerateAction->setEnabled(false);
    this->OpenProjectButton->setEnabled(false);
    this->ConfigureButton->setText(tr("&Stop"));
  } else if (s == Generating) {
    this->CacheModified = false;
    this->setEnabledState(false);
    this->ConfigureButton->setEnabled(false);
    this->GenerateAction->setEnabled(false);
    this->OpenProjectButton->setEnabled(false);
    this->GenerateButton->setText(tr("&Stop"));
  } else if (s == ReadyConfigure || s == ReadyGenerate) {
    this->setEnabledState(true);
    this->ConfigureButton->setText(tr("&Configure"));
    this->GenerateButton->setText(tr("&Generate"));
    this->updateCommandState();
  }
}

void RBLXBuildSetupDialog::updateCommandState()
{
  bool const pending = this->PresetApplicationPending;

  switch (this->CurrentState) {
    case Interrupting:
      this->ConfigureButton->setEnabled(false);
      this->GenerateButton->setEnabled(false);
      this->ConfigureAction->setEnabled(false);
      this->GenerateAction->setEnabled(false);
      this->OpenProjectButton->setEnabled(false);
      break;
    case Configuring:
      this->ConfigureButton->setEnabled(true); // acts as Stop
      this->GenerateButton->setEnabled(false);
      this->ConfigureAction->setEnabled(false);
      this->GenerateAction->setEnabled(false);
      this->OpenProjectButton->setEnabled(false);
      break;
    case Generating:
      this->GenerateButton->setEnabled(true); // acts as Stop
      this->ConfigureButton->setEnabled(false);
      this->ConfigureAction->setEnabled(false);
      this->GenerateAction->setEnabled(false);
      this->OpenProjectButton->setEnabled(false);
      break;
    case ReadyConfigure:
    case ReadyGenerate: {
      bool const enabled = !pending;
      this->ConfigureButton->setEnabled(enabled);
      this->GenerateButton->setEnabled(enabled);
      this->ConfigureAction->setEnabled(enabled);
      this->GenerateAction->setEnabled(enabled);

      // Gate everything that reads or mutates the still-stale model, but keep
      // the combo live so the user can pick a different preset.
      this->CacheValues->cacheModel()->setEditEnabled(enabled);
      this->SourceDirectory->setEnabled(enabled);
      this->BrowseSourceDirectoryButton->setEnabled(enabled);
      this->BinaryDirectory->setEnabled(enabled);
      this->BrowseBinaryDirectoryButton->setEnabled(enabled);
      this->ReloadCacheAction->setEnabled(enabled);
      this->DeleteCacheAction->setEnabled(enabled);
      this->ReloadPresetsButton->setEnabled(enabled);
      this->AddEntry->setEnabled(enabled);
      this->Environment->setEnabled(enabled);
      this->Preset->setEnabled(!this->Preset->presets().isEmpty());
      if (enabled) {
        this->selectionChanged(); // let selection re-enable Remove
      } else {
        this->RemoveEntry->setEnabled(false);
      }
      break;
    }
  }
}

void RBLXBuildSetupDialog::editEnvironment()
{
  EnvironmentDialog dialog(this->RBLXBuildThread->rblxbuildInstance()->environment(),
                           this);
  if (dialog.exec() == QDialog::Accepted) {
    QMetaObject::invokeMethod(
      this->RBLXBuildThread->rblxbuildInstance(), "setEnvironment",
      Q_ARG(QProcessEnvironment, dialog.environment()));
  }
}

void RBLXBuildSetupDialog::addCacheEntry()
{
  QDialog dialog(this);
  dialog.resize(400, 200);
  dialog.setWindowTitle(tr("Add Cache Entry"));
  QVBoxLayout* l = new QVBoxLayout(&dialog);
  AddCacheEntry* w =
    new AddCacheEntry(&dialog, this->AddVariableNames, this->AddVariableTypes);
  QDialogButtonBox* btns = new QDialogButtonBox(
    QDialogButtonBox::Ok | QDialogButtonBox::Cancel, Qt::Horizontal, &dialog);
  QObject::connect(btns, &QDialogButtonBox::accepted, &dialog,
                   &QDialog::accept);
  QObject::connect(btns, &QDialogButtonBox::rejected, &dialog,
                   &QDialog::reject);
  l->addWidget(w);
  l->addStretch();
  l->addWidget(btns);
  if (QDialog::Accepted == dialog.exec()) {
    QRBLXBuildCacheModel* m = this->CacheValues->cacheModel();
    m->insertProperty(w->type(), w->name(), w->description(), w->value(),
                      false);

    // only add variable names to the completion which are new
    if (!this->AddVariableNames.contains(w->name())) {
      this->AddVariableNames << w->name();
      this->AddVariableTypes << w->typeString();
      // limit to at most 100 completion items
      if (this->AddVariableNames.size() > 100) {
        this->AddVariableNames.removeFirst();
        this->AddVariableTypes.removeFirst();
      }
      // make sure RBLXBUILD_INSTALL_PREFIX is always there
      if (!this->AddVariableNames.contains("RBLXBUILD_INSTALL_PREFIX")) {
        this->AddVariableNames << "RBLXBUILD_INSTALL_PREFIX";
        this->AddVariableTypes << "PATH";
      }
      QSettings settings;
      settings.beginGroup("Settings/StartPath");
      settings.setValue("AddVariableNames", this->AddVariableNames);
      settings.setValue("AddVariableTypes", this->AddVariableTypes);
    }
  }
}

void RBLXBuildSetupDialog::startSearch()
{
  this->Search->setFocus(Qt::OtherFocusReason);
  this->Search->selectAll();
}

void RBLXBuildSetupDialog::setDebugOutput(bool flag)
{
  QMetaObject::invokeMethod(this->RBLXBuildThread->rblxbuildInstance(),
                            "setDebugOutput", Qt::QueuedConnection,
                            Q_ARG(bool, flag));
}

void RBLXBuildSetupDialog::setGroupedView(bool v)
{
  this->CacheValues->cacheModel()->setViewType(v ? QRBLXBuildCacheModel::GroupView
                                                 : QRBLXBuildCacheModel::FlatView);
  this->CacheValues->setRootIsDecorated(v);

  QSettings settings;
  settings.beginGroup("Settings/StartPath");
  settings.setValue("GroupView", v);
}

void RBLXBuildSetupDialog::setAdvancedView(bool v)
{
  this->CacheValues->setShowAdvanced(v);
  QSettings settings;
  settings.beginGroup("Settings/StartPath");
  settings.setValue("AdvancedView", v);
}

void RBLXBuildSetupDialog::showUserChanges()
{
  QSet<QRBLXBuildProperty> changes =
    qobject_cast<QRBLXBuildCacheModelDelegate*>(this->CacheValues->itemDelegate())
      ->changes();

  QDialog dialog(this);
  dialog.setWindowTitle(tr("My Changes"));
  dialog.resize(600, 400);
  QVBoxLayout* l = new QVBoxLayout(&dialog);
  QTextEdit* textedit = new QTextEdit(&dialog);
  textedit->setReadOnly(true);
  l->addWidget(textedit);
  QDialogButtonBox* btns =
    new QDialogButtonBox(QDialogButtonBox::Close, Qt::Horizontal, &dialog);
  QObject::connect(btns, &QDialogButtonBox::rejected, &dialog,
                   &QDialog::accept);
  l->addWidget(btns);

  QString command;
  QString cache;

  foreach (QRBLXBuildProperty const& prop, changes) {
    QString type;
    switch (prop.Type) {
      case QRBLXBuildProperty::BOOL:
        type = "BOOL";
        break;
      case QRBLXBuildProperty::PATH:
        type = "PATH";
        break;
      case QRBLXBuildProperty::FILEPATH:
        type = "FILEPATH";
        break;
      case QRBLXBuildProperty::STRING:
        type = "STRING";
        break;
    }
    QString value;
    if (prop.Type == QRBLXBuildProperty::BOOL) {
      value = prop.Value.toBool() ? "1" : "0";
    } else {
      value = prop.Value.toString();
    }

    QString const line = QString("%1:%2=").arg(prop.Key, type);
    command += QString("-D%1\"%2\" ").arg(line, value);
    cache += QString("%1%2\n").arg(line, value);
  }

  textedit->append(tr("Commandline options:"));
  textedit->append(command);
  textedit->append("\n");
  textedit->append(tr("Cache file:"));
  textedit->append(cache);

  dialog.exec();
}

void RBLXBuildSetupDialog::setSearchFilter(QString const& str)
{
  this->CacheValues->selectionModel()->clear();
  bool const valid = this->CacheValues->setSearchFilter(str);
  QtRBLXBuild::setSearchFilterColor(this->Search, valid);
}

void RBLXBuildSetupDialog::doOutputContextMenu(QPoint pt)
{
  std::unique_ptr<QMenu> menu(this->Output->createStandardContextMenu());

  menu->addSeparator();
  auto* a = menu->addAction(tr("Find..."));
  QObject::connect(a, &QAction::triggered, this,
                   &RBLXBuildSetupDialog::doOutputFindDialog);
  a->setShortcut(QKeySequence::Find);
  a = menu->addAction(tr("Find Next"));
  QObject::connect(a, &QAction::triggered, this,
                   &RBLXBuildSetupDialog::doOutputFindNext);
  a->setShortcut(QKeySequence::FindNext);
  a = menu->addAction(tr("Find Previous"));
  QObject::connect(a, &QAction::triggered, this,
                   &RBLXBuildSetupDialog::doOutputFindPrev);
  a->setShortcut(QKeySequence::FindPrevious);
  menu->addSeparator();
  a = menu->addAction(tr("Goto Next Error"));
  QObject::connect(a, &QAction::triggered, this,
                   &RBLXBuildSetupDialog::doOutputErrorNext);
  a->setShortcut(QKeySequence(Qt::Key_F8));

  menu->exec(this->Output->mapToGlobal(pt));
}

void RBLXBuildSetupDialog::doOutputFindDialog()
{
  QStringList strings(this->FindHistory);

  QString selection = this->Output->textCursor().selectedText();
  if (!selection.isEmpty() && !selection.contains(QChar::ParagraphSeparator) &&
      !selection.contains(QChar::LineSeparator)) {
    strings.push_front(selection);
  }

  bool ok;
  QString search = QInputDialog::getItem(this, tr("Find in Output"),
                                         tr("Find:"), strings, 0, true, &ok);
  if (ok && !search.isEmpty()) {
    if (!this->FindHistory.contains(search)) {
      this->FindHistory.push_front(search);
    }
    doOutputFindNext();
  }
}

void RBLXBuildSetupDialog::doRegexExplorerDialog()
{
  RegexExplorer dialog(this);
  dialog.exec();
}

void RBLXBuildSetupDialog::doOutputFindPrev()
{
  doOutputFindNext(false);
}

void RBLXBuildSetupDialog::doOutputFindNext(bool directionForward)
{
  if (this->FindHistory.isEmpty()) {
    doOutputFindDialog(); // will re-call this function again
    return;
  }

  QString search = this->FindHistory.front();

  QTextCursor textCursor = this->Output->textCursor();
  QTextDocument* document = this->Output->document();
  QTextDocument::FindFlags flags;
  if (!directionForward) {
    flags |= QTextDocument::FindBackward;
  }

  textCursor = document->find(search, textCursor, flags);

  if (textCursor.isNull()) {
    // first search found nothing, wrap around and search again
    textCursor = this->Output->textCursor();
    textCursor.movePosition(directionForward ? QTextCursor::Start
                                             : QTextCursor::End);
    textCursor = document->find(search, textCursor, flags);
  }

  if (textCursor.hasSelection()) {
    this->Output->setTextCursor(textCursor);
  }
}

void RBLXBuildSetupDialog::doOutputErrorNext()
{
  QTextCursor textCursor = this->Output->textCursor();
  bool atEnd = false;

  // move cursor out of current error-block
  if (textCursor.blockCharFormat() == this->ErrorFormat) {
    atEnd = !textCursor.movePosition(QTextCursor::NextBlock);
  }

  // move cursor to next error-block
  while (textCursor.blockCharFormat() != this->ErrorFormat && !atEnd) {
    atEnd = !textCursor.movePosition(QTextCursor::NextBlock);
  }

  if (atEnd) {
    // first search found nothing, wrap around and search again
    atEnd = !textCursor.movePosition(QTextCursor::Start);

    // move cursor to next error-block
    while (textCursor.blockCharFormat() != this->ErrorFormat && !atEnd) {
      atEnd = !textCursor.movePosition(QTextCursor::NextBlock);
    }
  }

  if (!atEnd) {
    textCursor.movePosition(QTextCursor::EndOfBlock, QTextCursor::KeepAnchor);

    QTextCharFormat selectionFormat;
    selectionFormat.setBackground(Qt::yellow);
    QTextEdit::ExtraSelection extraSelection = { textCursor, selectionFormat };
    this->Output->setExtraSelections(QList<QTextEdit::ExtraSelection>()
                                     << extraSelection);

    // make the whole error-block visible
    this->Output->setTextCursor(textCursor);

    // remove the selection to see the extraSelection
    textCursor.setPosition(textCursor.anchor());
    this->Output->setTextCursor(textCursor);
  }
}

void RBLXBuildSetupDialog::doWarningMessagesDialog()
{
  WarningMessagesDialog dialog(this, this->RBLXBuildThread->rblxbuildInstance());
  dialog.exec();
}
