/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file LICENSE.rst or https://linearbuild.org/licensing for details.  */
#include "LinearBuildSetupDialog.h"

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
#include "QLinearBuild.h"
#include "QLinearBuildCacheView.h"
#include "QLinearBuildSizeType.h"
#include "RegexExplorer.h"
#include "WarningMessagesDialog.h"

void OpenReferenceManual(QString const& filename)
{
  QString urlFormat("https://linearbuild.org/linearbuild/help/v%1.%2/");
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
  "LinearBuildPresets.json or LinearBuildUserPresets.json.";
}

QLinearBuildThread::QLinearBuildThread(QObject* p)
  : QThread(p)
{
}

QLinearBuild* QLinearBuildThread::linearbuildInstance() const
{
  return this->LinearBuildInstance.get();
}

void QLinearBuildThread::run()
{
  this->LinearBuildInstance = cm::make_unique<QLinearBuild>();
  // emit that this linearbuild thread is ready for use
  emit this->linearbuildInitialized();
  this->exec();
  this->LinearBuildInstance.reset();
}

LinearBuildSetupDialog::LinearBuildSetupDialog()
  : ExitAfterGenerate(true)
  , CacheModified(false)
  , ConfigureNeeded(true)
  , CurrentState(Interrupting)
{
  QString title = QString(tr("LinearBuild %1"));
  title = title.arg(cmVersion::GetLinearBuildVersion());
  this->setWindowTitle(title);

  // create the GUI
  QSettings settings;
  settings.beginGroup("Settings/StartPath");
  restoreGeometry(settings.value("geometry").toByteArray());
  restoreState(settings.value("windowState").toByteArray());

  this->AddVariableNames =
    settings.value("AddVariableNames", QStringList("LINEARBUILD_INSTALL_PREFIX"))
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
                   &LinearBuildSetupDialog::doReloadCache);
  this->DeleteCacheAction = FileMenu->addAction(tr("&Delete Cache"));
  QObject::connect(this->DeleteCacheAction, &QAction::triggered, this,
                   &LinearBuildSetupDialog::doDeleteCache);
  this->ExitAction = FileMenu->addAction(tr("E&xit"));
  QObject::connect(this->ExitAction, &QAction::triggered, this,
                   &LinearBuildSetupDialog::close);
  this->ExitAction->setShortcut(QKeySequence::Quit);

  QMenu* ToolsMenu = this->menuBar()->addMenu(tr("&Tools"));
  this->ConfigureAction = ToolsMenu->addAction(tr("&Configure"));
  QObject::connect(this->ConfigureAction, &QAction::triggered, this,
                   &LinearBuildSetupDialog::doConfigure);
  // prevent merging with Preferences menu item on macOS
  this->ConfigureAction->setMenuRole(QAction::NoRole);
  this->GenerateAction = ToolsMenu->addAction(tr("&Generate"));
  QObject::connect(this->GenerateAction, &QAction::triggered, this,
                   &LinearBuildSetupDialog::doGenerate);
  auto* a = ToolsMenu->addAction(tr("&Show My Changes"));
  QObject::connect(a, &QAction::triggered, this,
                   &LinearBuildSetupDialog::showUserChanges);
#if defined(Q_WS_MAC) || defined(Q_OS_MAC)
  this->InstallForCommandLineAction =
    ToolsMenu->addAction(tr("&How to Install For Command Line Use"));
  QObject::connect(this->InstallForCommandLineAction, &QAction::triggered,
                   this, &LinearBuildSetupDialog::doInstallForCommandLine);
#endif
  ToolsMenu->addSeparator();
  a = ToolsMenu->addAction(tr("Regular Expression Explorer..."));
  QObject::connect(a, &QAction::triggered, this,
                   &LinearBuildSetupDialog::doRegexExplorerDialog);
  ToolsMenu->addSeparator();
  a = ToolsMenu->addAction(tr("&Find in Output..."));
  QObject::connect(a, &QAction::triggered, this,
                   &LinearBuildSetupDialog::doOutputFindDialog);
  a->setShortcut(QKeySequence::Find);
  a = ToolsMenu->addAction(tr("Find Next"));
  QObject::connect(a, &QAction::triggered, this,
                   &LinearBuildSetupDialog::doOutputFindNext);
  a->setShortcut(QKeySequence::FindNext);
  a = ToolsMenu->addAction(tr("Find Previous"));
  QObject::connect(a, &QAction::triggered, this,
                   &LinearBuildSetupDialog::doOutputFindPrev);
  a->setShortcut(QKeySequence::FindPrevious);
  a = ToolsMenu->addAction(tr("Goto Next Error")); // in Visual Studio
  QObject::connect(a, &QAction::triggered, this,
                   &LinearBuildSetupDialog::doOutputErrorNext);
  a->setShortcut(QKeySequence(Qt::Key_F8));
  auto* s = new QShortcut(this);
#if (QT_VERSION < QT_VERSION_CHECK(6, 0, 0))
  s->setKey(QKeySequence(Qt::CTRL + Qt::Key_Period));
#else
  s->setKey(QKeySequence(Qt::CTRL | Qt::Key_Period));
#endif
  QObject::connect(s, &QShortcut::activated, this,
                   &LinearBuildSetupDialog::doOutputErrorNext); // in Eclipse

  QMenu* OptionsMenu = this->menuBar()->addMenu(tr("&Options"));
  a = OptionsMenu->addAction(tr("Warning Messages..."));
  QObject::connect(a, &QAction::triggered, this,
                   &LinearBuildSetupDialog::doWarningMessagesDialog);

  QAction* debugAction = OptionsMenu->addAction(tr("&Debug Output"));
  debugAction->setCheckable(true);
  QObject::connect(debugAction, &QAction::toggled, this,
                   &LinearBuildSetupDialog::setDebugOutput);

  OptionsMenu->addSeparator();
  a = OptionsMenu->addAction(tr("&Expand Grouped Entries"));
  QObject::connect(a, &QAction::triggered, this->CacheValues,
                   &QLinearBuildCacheView::expandAll);
  a = OptionsMenu->addAction(tr("&Collapse Grouped Entries"));
  QObject::connect(a, &QAction::triggered, this->CacheValues,
                   &QLinearBuildCacheView::collapseAll);

  QMenu* HelpMenu = this->menuBar()->addMenu(tr("&Help"));
  a = HelpMenu->addAction(tr("Help"));
  QObject::connect(a, &QAction::triggered, this, &LinearBuildSetupDialog::doHelp);
  a->setShortcut(QKeySequence::HelpContents);
  a = HelpMenu->addAction(tr("LinearBuild Reference Manual"));
  QObject::connect(a, &QAction::triggered, this,
                   [] { OpenReferenceManual("index.html"); });
  a = HelpMenu->addAction(tr("About"));
  QObject::connect(a, &QAction::triggered, this, &LinearBuildSetupDialog::doAbout);

  this->setAcceptDrops(true);

  // get the saved binary directories
  QStringList buildPaths = this->loadBuildPaths();
  this->BinaryDirectory->addItems(buildPaths);

  this->BinaryDirectory->setCompleter(new QLinearBuildFileCompleter(this, true));
  this->SourceDirectory->setCompleter(new QLinearBuildFileCompleter(this, true));

  // fixed pitch font in output window
  QFont outputFont("Courier New");
  outputFont.setStyleHint(QFont::Monospace);
  this->Output->setFont(outputFont);
  this->ErrorFormat.setForeground(QBrush(Qt::red));

  this->Output->setContextMenuPolicy(Qt::CustomContextMenu);
  connect(this->Output, &QTextEdit::customContextMenuRequested, this,
          &LinearBuildSetupDialog::doOutputContextMenu);

  // disable open project button
  this->OpenProjectButton->setDisabled(true);

  // start the linearbuild worker thread
  this->LinearBuildThread = new QLinearBuildThread(this);
  QObject::connect(this->LinearBuildThread, &QLinearBuildThread::linearbuildInitialized, this,
                   &LinearBuildSetupDialog::initialize, Qt::QueuedConnection);
  this->LinearBuildThread->start();

  this->enterState(ReadyConfigure);

  ProgressOffset = 0.0;
  ProgressFactor = 1.0;
}

void LinearBuildSetupDialog::initialize()
{
  // now the linearbuild worker thread is running, lets make our connections to it
  QObject::connect(this->LinearBuildThread->linearbuildInstance(),
                   &QLinearBuild::propertiesChanged, this->CacheValues->cacheModel(),
                   &QLinearBuildCacheModel::setProperties);

  QObject::connect(this->ConfigureButton, &QAbstractButton::clicked, this,
                   &LinearBuildSetupDialog::doConfigure);

  QObject::connect(this->LinearBuildThread->linearbuildInstance(), &QLinearBuild::configureDone,
                   this, &LinearBuildSetupDialog::exitLoop);
  QObject::connect(this->LinearBuildThread->linearbuildInstance(), &QLinearBuild::generateDone,
                   this, &LinearBuildSetupDialog::exitLoop);

  QObject::connect(this->GenerateButton, &QAbstractButton::clicked, this,
                   &LinearBuildSetupDialog::doGenerate);
  QObject::connect(this->OpenProjectButton, &QAbstractButton::clicked, this,
                   &LinearBuildSetupDialog::doOpenProject);

  QObject::connect(this->BrowseSourceDirectoryButton,
                   &QAbstractButton::clicked, this,
                   &LinearBuildSetupDialog::doSourceBrowse);
  QObject::connect(this->ReloadPresetsButton, &QAbstractButton::clicked,
                   this->LinearBuildThread->linearbuildInstance(), &QLinearBuild::loadPresets);
  QObject::connect(this->BrowseBinaryDirectoryButton,
                   &QAbstractButton::clicked, this,
                   &LinearBuildSetupDialog::doBinaryBrowse);

  QObject::connect(this->BinaryDirectory, &QComboBox::editTextChanged, this,
                   &LinearBuildSetupDialog::onBinaryDirectoryChanged);
  QObject::connect(this->SourceDirectory, &QLineEdit::textChanged, this,
                   &LinearBuildSetupDialog::onSourceDirectoryChanged);
  QObject::connect(this->Preset, &QLinearBuildPresetComboBox::presetChanged, this,
                   &LinearBuildSetupDialog::onBuildPresetChanged);

  QObject::connect(this->LinearBuildThread->linearbuildInstance(),
                   &QLinearBuild::sourceDirChanged, this,
                   &LinearBuildSetupDialog::updateSourceDirectory);
  QObject::connect(this->LinearBuildThread->linearbuildInstance(),
                   &QLinearBuild::binaryDirChanged, this,
                   &LinearBuildSetupDialog::updateBinaryDirectory);
  QObject::connect(this->LinearBuildThread->linearbuildInstance(), &QLinearBuild::presetsChanged,
                   this, &LinearBuildSetupDialog::updatePresets);
  QObject::connect(this->LinearBuildThread->linearbuildInstance(), &QLinearBuild::presetChanged,
                   this, &LinearBuildSetupDialog::updatePreset);
  QObject::connect(this->LinearBuildThread->linearbuildInstance(), &QLinearBuild::presetApplied,
                   this, &LinearBuildSetupDialog::onPresetApplied);
  QObject::connect(this->LinearBuildThread->linearbuildInstance(),
                   &QLinearBuild::presetLoadError, this,
                   &LinearBuildSetupDialog::showPresetLoadError);

  QObject::connect(this->LinearBuildThread->linearbuildInstance(),
                   &QLinearBuild::progressChanged, this,
                   &LinearBuildSetupDialog::showProgress);

  QObject::connect(this->LinearBuildThread->linearbuildInstance(), &QLinearBuild::errorMessage,
                   this, &LinearBuildSetupDialog::error);

  QObject::connect(this->LinearBuildThread->linearbuildInstance(), &QLinearBuild::outputMessage,
                   this, &LinearBuildSetupDialog::message);

  QObject::connect(this->LinearBuildThread->linearbuildInstance(), &QLinearBuild::openPossible,
                   this->OpenProjectButton, &LinearBuildSetupDialog::setEnabled);

  QObject::connect(this->groupedCheck, &QCheckBox::toggled, this,
                   &LinearBuildSetupDialog::setGroupedView);
  QObject::connect(this->advancedCheck, &QCheckBox::toggled, this,
                   &LinearBuildSetupDialog::setAdvancedView);
  QObject::connect(this->Search, &QLineEdit::textChanged, this,
                   &LinearBuildSetupDialog::setSearchFilter);

  QObject::connect(this->LinearBuildThread->linearbuildInstance(),
                   &QLinearBuild::generatorChanged, this,
                   &LinearBuildSetupDialog::updateGeneratorLabel);
  this->updateGeneratorLabel(QString());

  QObject::connect(this->CacheValues->cacheModel(),
                   &QLinearBuildCacheModel::dataChanged, this,
                   &LinearBuildSetupDialog::setCacheModified);

  QObject::connect(this->CacheValues->selectionModel(),
                   &QItemSelectionModel::selectionChanged, this,
                   &LinearBuildSetupDialog::selectionChanged);
  QObject::connect(this->RemoveEntry, &QAbstractButton::clicked, this,
                   &LinearBuildSetupDialog::removeSelectedCacheEntries);
  QObject::connect(this->AddEntry, &QAbstractButton::clicked, this,
                   &LinearBuildSetupDialog::addCacheEntry);

  QObject::connect(this->Environment, &QAbstractButton::clicked, this,
                   &LinearBuildSetupDialog::editEnvironment);

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

LinearBuildSetupDialog::~LinearBuildSetupDialog()
{
  QSettings settings;
  settings.beginGroup("Settings/StartPath");
  settings.setValue("windowState", QVariant(saveState()));
  settings.setValue("geometry", QVariant(saveGeometry()));
  settings.setValue("SplitterSizes", this->Splitter->saveState());

  // wait for thread to stop
  this->LinearBuildThread->quit();
  this->LinearBuildThread->wait();
}

bool LinearBuildSetupDialog::prepareConfigure()
{
  // make sure build directory exists
  QString bindir = this->LinearBuildThread->linearbuildInstance()->binaryDirectory();
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
  if (this->LinearBuildThread->linearbuildInstance()->generator().isEmpty()) {
    if (!this->setupFirstConfigure()) {
      return false;
    }
  }

  // remember paths
  this->addBinaryPath(
    this->LinearBuildThread->linearbuildInstance()->relativeBinaryDirectory());
  QSettings settings;
  settings.beginGroup("Settings/StartPath");
  settings.setValue("WhereSource",
                    this->LinearBuildThread->linearbuildInstance()->sourceDirectory());

  return true;
}

void LinearBuildSetupDialog::exitLoop(int err)
{
  this->LocalLoop.exit(err);
}

void LinearBuildSetupDialog::doConfigure()
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

bool LinearBuildSetupDialog::doConfigureInternal()
{
  this->Output->clear();
  this->CacheValues->selectionModel()->clear();

  QMetaObject::invokeMethod(
    this->LinearBuildThread->linearbuildInstance(), "setProperties", Qt::QueuedConnection,
    Q_ARG(QLinearBuildPropertyList, this->CacheValues->cacheModel()->properties()));
  QMetaObject::invokeMethod(this->LinearBuildThread->linearbuildInstance(), "configure",
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

void LinearBuildSetupDialog::doInstallForCommandLine()
{
  QString title = tr("How to Install For Command Line Use");
  QString msg = tr("One may add LinearBuild to the PATH:\n"
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
    cmSystemTools::GetFilenamePath(cmSystemTools::GetLinearBuildCommand()).c_str());
  msg = msg.arg(cmSystemTools::GetLinearBuildGUICommand().c_str());
  msg = msg.arg(cmSystemTools::GetLinearBuildGUICommand().c_str());

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

bool LinearBuildSetupDialog::doGenerateInternal()
{
  QMetaObject::invokeMethod(this->LinearBuildThread->linearbuildInstance(), "generate",
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

void LinearBuildSetupDialog::doGenerate()
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

void LinearBuildSetupDialog::doOpenProject()
{
  QMetaObject::invokeMethod(this->LinearBuildThread->linearbuildInstance(), "open",
                            Qt::QueuedConnection);
}

void LinearBuildSetupDialog::closeEvent(QCloseEvent* e)
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

void LinearBuildSetupDialog::doHelp()
{
  QString msg = tr(
    "LinearBuild is used to configure and generate build files for "
    "software projects.   The basic steps for configuring a project are as "
    "follows:\r\n\r\n1. Select the source directory for the project.  This "
    "should "
    "contain the LinearBuildLists.txt files for the project.\r\n\r\n2. Select the "
    "build "
    "directory for the project.   This is the directory where the project "
    "will be "
    "built.  It can be the same or a different directory than the source "
    "directory.   For easy clean up, a separate build directory is "
    "recommended. "
    "LinearBuild will create the directory if it does not exist.\r\n\r\n3. Once the "
    "source and binary directories are selected, it is time to press the "
    "Configure button.  This will cause LinearBuild to read all of the input files "
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

void LinearBuildSetupDialog::doInterrupt()
{
  this->enterState(Interrupting);
  this->LinearBuildThread->linearbuildInstance()->interrupt();
}

void LinearBuildSetupDialog::doSourceBrowse()
{
  QString dir = QFileDialog::getExistingDirectory(
    this, tr("Enter Path to Source"), this->SourceDirectory->text(),
    QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
  if (!dir.isEmpty()) {
    this->setSourceDirectory(dir);
  }
}

void LinearBuildSetupDialog::updateSourceDirectory(QString const& dir)
{
  if (this->SourceDirectory->text() != dir) {
    this->SourceDirectory->blockSignals(true);
    this->SourceDirectory->setText(dir);
    this->SourceDirectory->blockSignals(false);
  }
}

void LinearBuildSetupDialog::updateBinaryDirectory(QString const& dir)
{
  if (this->BinaryDirectory->currentText() != dir) {
    this->BinaryDirectory->blockSignals(true);
    this->BinaryDirectory->setEditText(dir);
    this->BinaryDirectory->blockSignals(false);
  }
}

void LinearBuildSetupDialog::updatePresets(QVector<QLinearBuildPreset> const& presets)
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

void LinearBuildSetupDialog::onPresetApplied(quint64 requestId, QString const& name,
                                       QLinearBuildPropertyList const& properties)
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

void LinearBuildSetupDialog::updatePreset(QString const& name)
{
  if (this->Preset->presetName() != name) {
    this->Preset->blockSignals(true);
    this->Preset->setPresetName(name);
    this->Preset->blockSignals(false);
  }
}

void LinearBuildSetupDialog::showPresetLoadError(QString const& dir,
                                           QString const& message)
{
  QMessageBox::warning(
    this, "Error Reading LinearBuild Presets",
    QString("Could not read presets from %1: %2").arg(dir, message));
}

void LinearBuildSetupDialog::doBinaryBrowse()
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

void LinearBuildSetupDialog::setBinaryDirectory(QString const& dir)
{
  this->BinaryDirectory->setEditText(dir);
}

void LinearBuildSetupDialog::setStartupBinaryDirectory(bool startup)
{
  this->StartupBinaryDirectory = startup;
}

void LinearBuildSetupDialog::onSourceDirectoryChanged(QString const& dir)
{
  this->Output->clear();
  QMetaObject::invokeMethod(this->LinearBuildThread->linearbuildInstance(),
                            "setSourceDirectory", Qt::QueuedConnection,
                            Q_ARG(QString, dir));
}

void LinearBuildSetupDialog::onBinaryDirectoryChanged(QString const& dir)
{
  QString title = QString(tr("LinearBuild %1 - %2"));
  title = title.arg(cmVersion::GetLinearBuildVersion());
  title = title.arg(dir);
  this->setWindowTitle(title);

  this->CacheModified = false;
  this->CacheValues->cacheModel()->clear();
  qobject_cast<QLinearBuildCacheModelDelegate*>(this->CacheValues->itemDelegate())
    ->clearChanges();
  this->Output->clear();
  QMetaObject::invokeMethod(this->LinearBuildThread->linearbuildInstance(),
                            "setBinaryDirectory", Qt::QueuedConnection,
                            Q_ARG(QString, dir));
}

void LinearBuildSetupDialog::onBuildPresetChanged(QString const& name)
{
  // Applying is async: gate the dependent actions now, until the completion
  // lands.
  quint64 const requestId = ++this->LatestPresetRequestId;
  this->PresetApplicationPending = true;
  // First Generate after an apply must reconfigure with the new inputs.
  this->ConfigureNeeded = true;
  this->updateCommandState();

  QMetaObject::invokeMethod(this->LinearBuildThread->linearbuildInstance(), "applyPreset",
                            Qt::QueuedConnection, Q_ARG(QString, name),
                            Q_ARG(bool, !this->StartupBinaryDirectory),
                            Q_ARG(quint64, requestId));
  this->StartupBinaryDirectory = false;
}

void LinearBuildSetupDialog::setSourceDirectory(QString const& dir)
{
  this->SourceDirectory->setText(dir);
}

void LinearBuildSetupDialog::setDeferredPreset(QString const& preset)
{
  this->DeferredPreset = preset;
  // A --preset isn't applied until presets load; gate now so nothing runs
  // with default inputs during that startup window.
  if (!preset.isNull()) {
    this->PresetApplicationPending = true;
    this->updateCommandState();
  }
}

void LinearBuildSetupDialog::showProgress(QString const& /*msg*/, float percent)
{
  percent = (percent * ProgressFactor) + ProgressOffset;
  this->ProgressBar->setValue(qRound(percent * 100));

#ifdef QT_WINEXTRAS
  QWinTaskbarProgress* progress = this->TaskbarButton->progress();
  progress->setVisible(true);
  progress->setValue(qRound(percent * 100));
#endif
}

void LinearBuildSetupDialog::error(QString const& msg)
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

void LinearBuildSetupDialog::message(QString const& msg)
{
  this->Output->setCurrentCharFormat(this->MessageFormat);
  this->Output->append(msg);
}

void LinearBuildSetupDialog::setEnabledState(bool enabled)
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

bool LinearBuildSetupDialog::setupFirstConfigure()
{
  FirstConfigure dialog;

  // initialize dialog and restore saved settings

  // add generators
  dialog.setGenerators(
    this->LinearBuildThread->linearbuildInstance()->availableGenerators());

  // restore from settings
  dialog.loadFromSettings();

  auto presetData = this->Preset->currentData();
  if (presetData.isValid()) {
    auto preset = presetData.value<QLinearBuildPreset>();
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
    this->LinearBuildThread->linearbuildInstance()->setGenerator(dialog.getGenerator());
    this->LinearBuildThread->linearbuildInstance()->setPlatform(dialog.getPlatform());
    this->LinearBuildThread->linearbuildInstance()->setToolset(dialog.getToolset());

    QLinearBuildCacheModel* m = this->CacheValues->cacheModel();

    if (dialog.compilerSetup()) {
      QString fortranCompiler = dialog.getFortranCompiler();
      if (!fortranCompiler.isEmpty()) {
        m->insertProperty(QLinearBuildProperty::FILEPATH, "LINEARBUILD_Fortran_COMPILER",
                          "Fortran compiler.", fortranCompiler, false);
      }
      QString cxxCompiler = dialog.getCXXCompiler();
      if (!cxxCompiler.isEmpty()) {
        m->insertProperty(QLinearBuildProperty::FILEPATH, "LINEARBUILD_CXX_COMPILER",
                          "CXX compiler.", cxxCompiler, false);
      }

      QString cCompiler = dialog.getCCompiler();
      if (!cCompiler.isEmpty()) {
        m->insertProperty(QLinearBuildProperty::FILEPATH, "LINEARBUILD_C_COMPILER",
                          "C compiler.", cCompiler, false);
      }
    } else if (dialog.crossCompilerSetup()) {
      QString fortranCompiler = dialog.getFortranCompiler();
      if (!fortranCompiler.isEmpty()) {
        m->insertProperty(QLinearBuildProperty::FILEPATH, "LINEARBUILD_Fortran_COMPILER",
                          "Fortran compiler.", fortranCompiler, false);
      }

      QString mode = dialog.getCrossIncludeMode();
      m->insertProperty(QLinearBuildProperty::STRING,
                        "LINEARBUILD_FIND_ROOT_PATH_MODE_INCLUDE",
                        tr("LinearBuild Find Include Mode"), mode, false);
      mode = dialog.getCrossLibraryMode();
      m->insertProperty(QLinearBuildProperty::STRING,
                        "LINEARBUILD_FIND_ROOT_PATH_MODE_LIBRARY",
                        tr("LinearBuild Find Library Mode"), mode, false);
      mode = dialog.getCrossProgramMode();
      m->insertProperty(QLinearBuildProperty::STRING,
                        "LINEARBUILD_FIND_ROOT_PATH_MODE_PROGRAM",
                        tr("LinearBuild Find Program Mode"), mode, false);

      QString rootPath = dialog.getCrossRoot();
      m->insertProperty(QLinearBuildProperty::PATH, "LINEARBUILD_FIND_ROOT_PATH",
                        tr("LinearBuild Find Root Path"), rootPath, false);

      QString systemName = dialog.getSystemName();
      m->insertProperty(QLinearBuildProperty::STRING, "LINEARBUILD_SYSTEM_NAME",
                        tr("LinearBuild System Name"), systemName, false);
      QString systemVersion = dialog.getSystemVersion();
      m->insertProperty(QLinearBuildProperty::STRING, "LINEARBUILD_SYSTEM_VERSION",
                        tr("LinearBuild System Version"), systemVersion, false);
      QString systemProcessor = dialog.getSystemProcessor();
      m->insertProperty(QLinearBuildProperty::STRING, "LINEARBUILD_SYSTEM_PROCESSOR",
                        tr("LinearBuild System Processor"), systemProcessor, false);
      QString cxxCompiler = dialog.getCXXCompiler();
      if (!cxxCompiler.isEmpty()) {
        m->insertProperty(QLinearBuildProperty::FILEPATH, "LINEARBUILD_CXX_COMPILER",
                          tr("CXX compiler."), cxxCompiler, false);
      }
      QString cCompiler = dialog.getCCompiler();
      if (!cCompiler.isEmpty()) {
        m->insertProperty(QLinearBuildProperty::FILEPATH, "LINEARBUILD_C_COMPILER",
                          tr("C compiler."), cCompiler, false);
      }
    } else if (dialog.crossCompilerToolChainFile()) {
      QString toolchainFile = dialog.getCrossCompilerToolChainFile();
      m->insertProperty(QLinearBuildProperty::FILEPATH, "LINEARBUILD_TOOLCHAIN_FILE",
                        tr("Cross Compile ToolChain File"), toolchainFile,
                        false);
    }
    return true;
  }

  return false;
}

void LinearBuildSetupDialog::updateGeneratorLabel(QString const& gen)
{
  QString str = tr("Current Generator: ");
  if (gen.isEmpty()) {
    str += tr("None");
  } else {
    str += gen;
  }
  this->Generator->setText(str);
}

void LinearBuildSetupDialog::doReloadCache()
{
  QMetaObject::invokeMethod(this->LinearBuildThread->linearbuildInstance(), "reloadCache",
                            Qt::QueuedConnection);
}

void LinearBuildSetupDialog::doDeleteCache()
{
  QString title = tr("Delete Cache");
  QString msg = tr("Are you sure you want to delete the cache?");
  QMessageBox::StandardButton btn;
  btn = QMessageBox::information(this, title, msg,
                                 QMessageBox::Yes | QMessageBox::No);
  if (btn == QMessageBox::No) {
    return;
  }
  QMetaObject::invokeMethod(this->LinearBuildThread->linearbuildInstance(), "deleteCache",
                            Qt::QueuedConnection);
}

void LinearBuildSetupDialog::doAbout()
{
  QString msg = tr(
    "LinearBuild %1 (linearbuild.org).\n"
    "LinearBuild suite maintained and supported by Kitware (kitware.com/linearbuild).\n"
    "Distributed under terms of the BSD 3-Clause License.\n"
    "\n"
    "LinearBuild GUI built using Qt %2 (qt-project.org).\n"
#ifdef USE_LGPL
    "\n"
    "The Qt Toolkit is Copyright (C) The Qt Company Ltd.\n"
    "Qt is licensed under terms of the GNU LGPLv" USE_LGPL ", available at:\n"
    " \"%3\""
#endif
  );
  msg = msg.arg(cmVersion::GetLinearBuildVersion());
  msg = msg.arg(qVersion());
#ifdef USE_LGPL
  std::string lgpl =
    cmSystemTools::GetLinearBuildRoot() + "/Licenses/LGPLv" USE_LGPL ".txt";
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

void LinearBuildSetupDialog::setExitAfterGenerate(bool b)
{
  this->ExitAfterGenerate = b;
}

void LinearBuildSetupDialog::addBinaryPath(QString const& path)
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

void LinearBuildSetupDialog::dragEnterEvent(QDragEnterEvent* e)
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
      (file.endsWith("LinearBuildCache.txt", Qt::CaseInsensitive) ||
       file.endsWith("LinearBuildLists.txt", Qt::CaseInsensitive))) {
    e->accept();
  } else {
    e->ignore();
  }
}

void LinearBuildSetupDialog::dropEvent(QDropEvent* e)
{
  if (!(this->CurrentState == ReadyConfigure ||
        this->CurrentState == ReadyGenerate)) {
    return;
  }

  QMimeData const* dat = e->mimeData();
  QList<QUrl> urls = dat->urls();
  QString file = urls.count() ? urls[0].toLocalFile() : QString();
  if (file.endsWith("LinearBuildCache.txt", Qt::CaseInsensitive)) {
    QFileInfo info(file);
    if (this->LinearBuildThread->linearbuildInstance()->binaryDirectory() !=
        info.absolutePath()) {
      this->setBinaryDirectory(info.absolutePath());
    }
  } else if (file.endsWith("LinearBuildLists.txt", Qt::CaseInsensitive)) {
    QFileInfo info(file);
    if (this->LinearBuildThread->linearbuildInstance()->binaryDirectory() !=
        info.absolutePath()) {
      this->setSourceDirectory(info.absolutePath());
      this->setBinaryDirectory(info.absolutePath());
    }
  }
}

QStringList LinearBuildSetupDialog::loadBuildPaths()
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

void LinearBuildSetupDialog::saveBuildPaths(QStringList const& paths)
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

void LinearBuildSetupDialog::setCacheModified()
{
  this->CacheModified = true;
  this->ConfigureNeeded = true;
  this->enterState(ReadyConfigure);
}

void LinearBuildSetupDialog::removeSelectedCacheEntries()
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

void LinearBuildSetupDialog::selectionChanged()
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

void LinearBuildSetupDialog::enterState(LinearBuildSetupDialog::State s)
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

void LinearBuildSetupDialog::updateCommandState()
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

void LinearBuildSetupDialog::editEnvironment()
{
  EnvironmentDialog dialog(this->LinearBuildThread->linearbuildInstance()->environment(),
                           this);
  if (dialog.exec() == QDialog::Accepted) {
    QMetaObject::invokeMethod(
      this->LinearBuildThread->linearbuildInstance(), "setEnvironment",
      Q_ARG(QProcessEnvironment, dialog.environment()));
  }
}

void LinearBuildSetupDialog::addCacheEntry()
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
    QLinearBuildCacheModel* m = this->CacheValues->cacheModel();
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
      // make sure LINEARBUILD_INSTALL_PREFIX is always there
      if (!this->AddVariableNames.contains("LINEARBUILD_INSTALL_PREFIX")) {
        this->AddVariableNames << "LINEARBUILD_INSTALL_PREFIX";
        this->AddVariableTypes << "PATH";
      }
      QSettings settings;
      settings.beginGroup("Settings/StartPath");
      settings.setValue("AddVariableNames", this->AddVariableNames);
      settings.setValue("AddVariableTypes", this->AddVariableTypes);
    }
  }
}

void LinearBuildSetupDialog::startSearch()
{
  this->Search->setFocus(Qt::OtherFocusReason);
  this->Search->selectAll();
}

void LinearBuildSetupDialog::setDebugOutput(bool flag)
{
  QMetaObject::invokeMethod(this->LinearBuildThread->linearbuildInstance(),
                            "setDebugOutput", Qt::QueuedConnection,
                            Q_ARG(bool, flag));
}

void LinearBuildSetupDialog::setGroupedView(bool v)
{
  this->CacheValues->cacheModel()->setViewType(v ? QLinearBuildCacheModel::GroupView
                                                 : QLinearBuildCacheModel::FlatView);
  this->CacheValues->setRootIsDecorated(v);

  QSettings settings;
  settings.beginGroup("Settings/StartPath");
  settings.setValue("GroupView", v);
}

void LinearBuildSetupDialog::setAdvancedView(bool v)
{
  this->CacheValues->setShowAdvanced(v);
  QSettings settings;
  settings.beginGroup("Settings/StartPath");
  settings.setValue("AdvancedView", v);
}

void LinearBuildSetupDialog::showUserChanges()
{
  QSet<QLinearBuildProperty> changes =
    qobject_cast<QLinearBuildCacheModelDelegate*>(this->CacheValues->itemDelegate())
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

  foreach (QLinearBuildProperty const& prop, changes) {
    QString type;
    switch (prop.Type) {
      case QLinearBuildProperty::BOOL:
        type = "BOOL";
        break;
      case QLinearBuildProperty::PATH:
        type = "PATH";
        break;
      case QLinearBuildProperty::FILEPATH:
        type = "FILEPATH";
        break;
      case QLinearBuildProperty::STRING:
        type = "STRING";
        break;
    }
    QString value;
    if (prop.Type == QLinearBuildProperty::BOOL) {
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

void LinearBuildSetupDialog::setSearchFilter(QString const& str)
{
  this->CacheValues->selectionModel()->clear();
  bool const valid = this->CacheValues->setSearchFilter(str);
  QtLinearBuild::setSearchFilterColor(this->Search, valid);
}

void LinearBuildSetupDialog::doOutputContextMenu(QPoint pt)
{
  std::unique_ptr<QMenu> menu(this->Output->createStandardContextMenu());

  menu->addSeparator();
  auto* a = menu->addAction(tr("Find..."));
  QObject::connect(a, &QAction::triggered, this,
                   &LinearBuildSetupDialog::doOutputFindDialog);
  a->setShortcut(QKeySequence::Find);
  a = menu->addAction(tr("Find Next"));
  QObject::connect(a, &QAction::triggered, this,
                   &LinearBuildSetupDialog::doOutputFindNext);
  a->setShortcut(QKeySequence::FindNext);
  a = menu->addAction(tr("Find Previous"));
  QObject::connect(a, &QAction::triggered, this,
                   &LinearBuildSetupDialog::doOutputFindPrev);
  a->setShortcut(QKeySequence::FindPrevious);
  menu->addSeparator();
  a = menu->addAction(tr("Goto Next Error"));
  QObject::connect(a, &QAction::triggered, this,
                   &LinearBuildSetupDialog::doOutputErrorNext);
  a->setShortcut(QKeySequence(Qt::Key_F8));

  menu->exec(this->Output->mapToGlobal(pt));
}

void LinearBuildSetupDialog::doOutputFindDialog()
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

void LinearBuildSetupDialog::doRegexExplorerDialog()
{
  RegexExplorer dialog(this);
  dialog.exec();
}

void LinearBuildSetupDialog::doOutputFindPrev()
{
  doOutputFindNext(false);
}

void LinearBuildSetupDialog::doOutputFindNext(bool directionForward)
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

void LinearBuildSetupDialog::doOutputErrorNext()
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

void LinearBuildSetupDialog::doWarningMessagesDialog()
{
  WarningMessagesDialog dialog(this, this->LinearBuildThread->linearbuildInstance());
  dialog.exec();
}
