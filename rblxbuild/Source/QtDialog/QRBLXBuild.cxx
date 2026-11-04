/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file LICENSE.rst or https://rblxbuild.org/licensing for details.  */
#include "QRBLXBuild.h"

#include <algorithm>

#include <cm/memory>

#include <QCoreApplication>
#include <QDir>
#include <QString>
#include <QVector>

#include "cmDiagnostics.h"
#include "cmExternalMakefileProjectGenerator.h"
#include "cmGlobalGenerator.h"
#include "cmMessageMetadata.h"
#include "cmState.h"
#include "cmStringAlgorithms.h"
#include "cmSystemTools.h"

#include "QRBLXBuildSizeType.h"

#ifdef Q_OS_WIN
#  include "qt_windows.h" // For SetErrorMode
#endif

static QString sanitizedDirPath(QString const& dir)
{
  QString result = QString::fromStdString(
    cmSystemTools::GetActualCaseForPath(dir.toStdString()));
  if (result.size() > 1 && result.endsWith(QDir::separator())) {
    result = result.left(result.size() - 1);
  }
  return result;
}

namespace {
// Holds a flag set for the duration of a scope; clears it even on early exit.
class ScopedFlag
{
public:
  explicit ScopedFlag(bool& flag)
    : Flag(flag)
  {
    this->Flag = true;
  }
  ~ScopedFlag() { this->Flag = false; }
  ScopedFlag(ScopedFlag const&) = delete;
  ScopedFlag& operator=(ScopedFlag const&) = delete;

private:
  bool& Flag;
};
}

QRBLXBuild::QRBLXBuild(QObject* p)
  : QObject(p)
  , StartEnvironment(QProcessEnvironment::systemEnvironment())
  , Environment(QProcessEnvironment::systemEnvironment())
{
  qRegisterMetaType<QRBLXBuildProperty>();
  qRegisterMetaType<QRBLXBuildPropertyList>();
  qRegisterMetaType<QProcessEnvironment>();
  qRegisterMetaType<QVector<QRBLXBuildPreset>>();
  qRegisterMetaType<quint64>("quint64");

  cmSystemTools::DisableRunCommandOutput();
  cmSystemTools::SetRunCommandHideConsole(true);

  cmSystemTools::SetMessageCallback(
    [this](std::string const& msg, cmMessageMetadata const& md) {
      this->messageCallback(msg, md.title);
    });
  cmSystemTools::SetStdoutCallback(
    [this](std::string const& msg) { this->stdoutCallback(msg); });
  cmSystemTools::SetStderrCallback(
    [this](std::string const& msg) { this->stderrCallback(msg); });

  this->RBLXBuildInstance = cm::make_unique<rblxbuild>(cmState::Role::Project);
  this->RBLXBuildInstance->SetRBLXBuildEditCommand(
    cmSystemTools::GetRBLXBuildGUICommand());
  this->RBLXBuildInstance->SetProgressCallback(
    [this](std::string const& msg, float percent) {
      this->progressCallback(msg, percent);
    });

  cmSystemTools::SetInterruptCallback(
    [this] { return this->interruptCallback(); });

  std::vector<rblxbuild::GeneratorInfo> generators;
  this->RBLXBuildInstance->GetRegisteredGenerators(generators);

  for (rblxbuild::GeneratorInfo const& gen : generators) {
    this->AvailableGenerators.push_back(gen);
  }
}

QRBLXBuild::~QRBLXBuild() = default;

void QRBLXBuild::loadCache(QString const& dir)
{
  this->setBinaryDirectory(dir);
}

void QRBLXBuild::setSourceDirectory(QString const& _dir)
{
  QString dir = sanitizedDirPath(_dir);
  if (this->SourceDirectory != dir) {
    this->SourceDirectory = QDir::fromNativeSeparators(dir);
    emit this->sourceDirChanged(this->SourceDirectory);
    this->loadPresets();
    this->setPreset(QString{});
    if (!this->MaybeRelativeBinaryDirectory.isEmpty() &&
        !cmSystemTools::FileIsFullPath(
          this->MaybeRelativeBinaryDirectory.toStdString())) {
      this->setBinaryDirectory(this->MaybeRelativeBinaryDirectory);
    }
  }
}

void QRBLXBuild::setBinaryDirectory(QString const& _dir)
{
  QString dir = sanitizedDirPath(_dir);

  QString absDir = dir;

  if (!cmSystemTools::FileIsFullPath(absDir.toStdString())) {
    if (!this->SourceDirectory.isEmpty()) {
      if (!this->SourceDirectory.endsWith("/")) {
        absDir = "/" + absDir;
      }
      absDir = this->SourceDirectory + absDir;
    }
  }

  if (this->BinaryDirectory != absDir ||
      this->MaybeRelativeBinaryDirectory != dir) {
    this->MaybeRelativeBinaryDirectory = QDir::fromNativeSeparators(dir);
    this->BinaryDirectory = QDir::fromNativeSeparators(absDir);
    emit this->binaryDirChanged(this->MaybeRelativeBinaryDirectory);
    cmState* state = this->RBLXBuildInstance->GetState();
    this->setGenerator(QString());
    this->setToolset(QString());
    this->setPlatform(QString());
    if (!this->RBLXBuildInstance->LoadCache(this->BinaryDirectory.toStdString())) {
      QDir testDir(this->BinaryDirectory);
      if (testDir.exists("RBLXBuildCache.txt")) {
        cmSystemTools::Error(
          "There is a RBLXBuildCache.txt file for the current binary "
          "tree but rblxbuild does not have permission to read it.  "
          "Please check the permissions of the directory you are trying to "
          "run RBLXBuild on.");
      }
    }

    QRBLXBuildPropertyList props = this->properties();
    if (!this->ApplyingPreset) {
      emit this->propertiesChanged(props);
    }
    cmValue homeDir = state->GetCacheEntryValue("RBLXBUILD_HOME_DIRECTORY");
    if (homeDir) {
      setSourceDirectory(QString(homeDir->c_str()));
    }
    cmValue gen = state->GetCacheEntryValue("RBLXBUILD_GENERATOR");
    if (gen) {
      cmValue extraGen =
        state->GetInitializedCacheValue("RBLXBUILD_EXTRA_GENERATOR");
      std::string curGen =
        cmExternalMakefileProjectGenerator::CreateFullGeneratorName(*gen,
                                                                    *extraGen);
      this->setGenerator(QString::fromStdString(curGen));
    }

    cmValue platform = state->GetCacheEntryValue("RBLXBUILD_GENERATOR_PLATFORM");
    if (platform) {
      this->setPlatform(QString(platform->c_str()));
    }

    cmValue toolset = state->GetCacheEntryValue("RBLXBUILD_GENERATOR_TOOLSET");
    if (toolset) {
      this->setToolset(QString(toolset->c_str()));
    }

    checkOpenPossible();
  }
}

void QRBLXBuild::setPreset(QString const& name, bool setBinary)
{
  if (this->PresetName != name) {
    this->PresetName = name;
    if (!this->ApplyingPreset) {
      emit this->presetChanged(this->PresetName);
    }

    if (!name.isNull()) {
      std::string presetName(name.toStdString());
      auto const expandedPreset =
        this->RBLXBuildPresetsGraph.ConfigurePresets[presetName].Expanded;
      if (expandedPreset) {
        if (setBinary && !expandedPreset->BinaryDir.empty()) {
          QString binaryDir =
            QString::fromStdString(expandedPreset->BinaryDir);
          this->setBinaryDirectory(binaryDir);
        }
        this->RBLXBuildInstance->SetDiagnosticsFromPreset(expandedPreset->Warnings,
                                                      expandedPreset->Errors);
        this->Environment = this->StartEnvironment;
        for (auto const& v : expandedPreset->Environment) {
          if (v.second) {
            this->Environment.insert(QString::fromStdString(v.first),
                                     QString::fromStdString(v.second.value()));
          }
        }
      }
    }
    if (!this->ApplyingPreset) {
      emit this->propertiesChanged(this->properties());
    }
  }
}

void QRBLXBuild::applyPreset(QString const& name, bool setBinary,
                         quint64 requestId)
{
  {
    // Silence the intermediate, untagged notifications so the completion below
    // is the only model update the UI trusts for this request.
    ScopedFlag applying(this->ApplyingPreset);
    this->setPreset(name, setBinary);
  }
  emit this->presetApplied(requestId, this->PresetName, this->properties());
}

void QRBLXBuild::setGenerator(QString const& gen)
{
  if (this->Generator != gen) {
    this->Generator = gen;
    emit this->generatorChanged(this->Generator);
  }
}

void QRBLXBuild::setPlatform(QString const& platform)
{
  if (this->Platform != platform) {
    this->Platform = platform;
    emit this->platformChanged(this->Platform);
  }
}

void QRBLXBuild::setToolset(QString const& toolset)
{
  if (this->Toolset != toolset) {
    this->Toolset = toolset;
    emit this->toolsetChanged(this->Toolset);
  }
}

void QRBLXBuild::setEnvironment(QProcessEnvironment const& environment)
{
  this->Environment = environment;
}

void QRBLXBuild::configure()
{
  int err;
  {
    cmSystemTools::SaveRestoreEnvironment restoreEnv;
    this->setUpEnvironment();

#ifdef Q_OS_WIN
    UINT lastErrorMode = SetErrorMode(0);
#endif
    // Apply the same transformations that the command-line invocation does
    auto sanitizePath = [](QString const& value) -> std::string {
      return cmSystemTools::ToNormalizedPathOnDisk(value.toStdString());
    };

    this->RBLXBuildInstance->SetHomeDirectory(sanitizePath(this->SourceDirectory));
    this->RBLXBuildInstance->SetHomeOutputDirectory(
      sanitizePath(this->BinaryDirectory));
    this->RBLXBuildInstance->SetGlobalGenerator(
      this->RBLXBuildInstance->CreateGlobalGenerator(
        this->Generator.toStdString()));
    this->RBLXBuildInstance->SetGeneratorPlatform(this->Platform.toStdString());
    this->RBLXBuildInstance->SetGeneratorToolset(this->Toolset.toStdString());
    this->RBLXBuildInstance->LoadCache();
    this->RBLXBuildInstance->PreLoadRBLXBuildFiles();

    InterruptFlag = 0;
    cmSystemTools::ResetErrorOccurredFlag();

    err = this->RBLXBuildInstance->Configure();

#ifdef Q_OS_WIN
    SetErrorMode(lastErrorMode);
#endif
  }

  emit this->propertiesChanged(this->properties());
  emit this->configureDone(err);
}

void QRBLXBuild::generate()
{
  int err;
  {
    cmSystemTools::SaveRestoreEnvironment restoreEnv;
    this->setUpEnvironment();

#ifdef Q_OS_WIN
    UINT lastErrorMode = SetErrorMode(0);
#endif

    InterruptFlag = 0;
    cmSystemTools::ResetErrorOccurredFlag();

    err = this->RBLXBuildInstance->Generate();

#ifdef Q_OS_WIN
    SetErrorMode(lastErrorMode);
#endif
  }

  emit this->generateDone(err);
  checkOpenPossible();
}

void QRBLXBuild::open()
{
#ifdef Q_OS_WIN
  UINT lastErrorMode = SetErrorMode(0);
#endif

  InterruptFlag = 0;
  cmSystemTools::ResetErrorOccurredFlag();

  auto successful = this->RBLXBuildInstance->Open(
    this->BinaryDirectory.toStdString(), rblxbuild::DryRun::No);

#ifdef Q_OS_WIN
  SetErrorMode(lastErrorMode);
#endif

  emit this->openDone(successful);
}

void QRBLXBuild::setProperties(QRBLXBuildPropertyList const& newProps)
{
  QRBLXBuildPropertyList props = newProps;

  QStringList toremove;

  // set the value of properties
  cmState* state = this->RBLXBuildInstance->GetState();
  std::vector<std::string> cacheKeys = state->GetCacheEntryKeys();
  for (std::string const& key : cacheKeys) {
    cmStateEnums::CacheEntryType t = state->GetCacheEntryType(key);
    if (t == cmStateEnums::INTERNAL || t == cmStateEnums::STATIC) {
      continue;
    }

    QRBLXBuildProperty prop;
    prop.Key = QString::fromStdString(key);
    cm_qsizetype idx = props.indexOf(prop);
    if (idx == -1) {
      toremove.append(QString::fromStdString(key));
    } else {
      prop = props[idx];
#if (QT_VERSION < QT_VERSION_CHECK(6, 0, 0))
      bool const isBool = prop.Value.type() == QVariant::Bool;
#else
      bool const isBool = prop.Value.metaType() == QMetaType::fromType<bool>();
#endif
      if (isBool) {
        state->SetCacheEntryValue(key, prop.Value.toBool() ? "ON" : "OFF");
      } else {
        state->SetCacheEntryValue(key, prop.Value.toString().toStdString());
      }
      props.removeAt(idx);
    }
  }

  // remove some properties
  foreach (QString const& s, toremove) {
    this->RBLXBuildInstance->UnwatchUnusedCli(s.toStdString());

    state->RemoveCacheEntry(s.toStdString());
  }

  // add some new properties
  foreach (QRBLXBuildProperty const& s, props) {
    this->RBLXBuildInstance->WatchUnusedCli(s.Key.toStdString());

    if (s.Type == QRBLXBuildProperty::BOOL) {
      this->RBLXBuildInstance->AddCacheEntry(
        s.Key.toStdString(), s.Value.toBool() ? "ON" : "OFF",
        s.Help.toStdString(), cmStateEnums::BOOL);
    } else if (s.Type == QRBLXBuildProperty::STRING) {
      this->RBLXBuildInstance->AddCacheEntry(
        s.Key.toStdString(), s.Value.toString().toStdString(),
        s.Help.toStdString(), cmStateEnums::STRING);
    } else if (s.Type == QRBLXBuildProperty::PATH) {
      this->RBLXBuildInstance->AddCacheEntry(
        s.Key.toStdString(), s.Value.toString().toStdString(),
        s.Help.toStdString(), cmStateEnums::PATH);
    } else if (s.Type == QRBLXBuildProperty::FILEPATH) {
      this->RBLXBuildInstance->AddCacheEntry(
        s.Key.toStdString(), s.Value.toString().toStdString(),
        s.Help.toStdString(), cmStateEnums::FILEPATH);
    }
  }

  this->RBLXBuildInstance->SaveCache(this->BinaryDirectory.toStdString());
}

namespace {
template <typename T>
QRBLXBuildProperty cache_to_property(T const& v)
{
  QRBLXBuildProperty prop;
  prop.Key = QString::fromStdString(v.first);
  prop.Value = QString::fromStdString(v.second->Value);
  prop.Type = QRBLXBuildProperty::STRING;
  if (!v.second->Type.empty()) {
    auto type = cmState::StringToCacheEntryType(v.second->Type);
    switch (type) {
      case cmStateEnums::BOOL:
        prop.Type = QRBLXBuildProperty::BOOL;
        prop.Value = cmIsOn(v.second->Value);
        break;
      case cmStateEnums::PATH:
        prop.Type = QRBLXBuildProperty::PATH;
        break;
      case cmStateEnums::FILEPATH:
        prop.Type = QRBLXBuildProperty::FILEPATH;
        break;
      default:
        prop.Type = QRBLXBuildProperty::STRING;
        break;
    }
  }
  return prop;
}

void add_to_property_list(QRBLXBuildPropertyList& list, QRBLXBuildProperty&& prop)
{
  // QRBLXBuildCacheModel prefers variables earlier in the list rather than
  // later, so overwrite them if they already exist rather than simply
  // appending
  bool found = false;
  for (auto& orig : list) {
    if (orig.Key == prop.Key) {
      orig = prop;
      found = true;
      break;
    }
  }
  if (!found) {
    list.append(prop);
  }
}
}

QRBLXBuildPropertyList QRBLXBuild::properties() const
{
  QRBLXBuildPropertyList ret;

  cmState* state = this->RBLXBuildInstance->GetState();
  std::vector<std::string> cacheKeys = state->GetCacheEntryKeys();
  for (std::string const& key : cacheKeys) {
    cmStateEnums::CacheEntryType t = state->GetCacheEntryType(key);
    if (t == cmStateEnums::INTERNAL || t == cmStateEnums::STATIC ||
        t == cmStateEnums::UNINITIALIZED) {
      continue;
    }

    cmValue cachedValue = state->GetCacheEntryValue(key);

    QRBLXBuildProperty prop;
    prop.Key = QString::fromStdString(key);
    if (cmValue hs = state->GetCacheEntryProperty(key, "HELPSTRING")) {
      prop.Help = QString(hs->c_str());
    }
    prop.Value = QString(cachedValue->c_str());
    prop.Advanced = state->GetCacheEntryPropertyAsBool(key, "ADVANCED");
    if (t == cmStateEnums::BOOL) {
      prop.Type = QRBLXBuildProperty::BOOL;
      prop.Value = cmIsOn(*cachedValue);
    } else if (t == cmStateEnums::PATH) {
      prop.Type = QRBLXBuildProperty::PATH;
    } else if (t == cmStateEnums::FILEPATH) {
      prop.Type = QRBLXBuildProperty::FILEPATH;
    } else if (t == cmStateEnums::STRING) {
      prop.Type = QRBLXBuildProperty::STRING;
      cmValue stringsProperty = state->GetCacheEntryProperty(key, "STRINGS");
      if (stringsProperty) {
        prop.Strings = QString(stringsProperty->c_str()).split(";");
      }
    }

    ret.append(prop);
  }

  if (!this->PresetName.isNull()) {
    std::string presetName(this->PresetName.toStdString());
    auto const& p =
      this->RBLXBuildPresetsGraph.ConfigurePresets.at(presetName).Expanded;
    if (p) {
      for (auto const& v : p->CacheVariables) {
        if (!v.second) {
          continue;
        }
        auto prop = cache_to_property(v);
        add_to_property_list(ret, std::move(prop));
      }
      // Dedicated fields override same-named cacheVariables, so apply last.
      if (!p->ToolchainFile.empty()) {
        using CacheVariable = cmRBLXBuildPresetsGraph::CacheVariable;
        CacheVariable var{ "FILEPATH", p->ToolchainFile };
        std::pair<std::string, cm::optional<CacheVariable>> value = {
          "RBLXBUILD_TOOLCHAIN_FILE", var
        };
        auto prop = cache_to_property(value);
        add_to_property_list(ret, std::move(prop));
      }
      if (!p->InstallDir.empty()) {
        using CacheVariable = cmRBLXBuildPresetsGraph::CacheVariable;
        CacheVariable var{ "PATH", p->InstallDir };
        std::pair<std::string, cm::optional<CacheVariable>> value = {
          "RBLXBUILD_INSTALL_PREFIX", var
        };
        auto prop = cache_to_property(value);
        add_to_property_list(ret, std::move(prop));
      }
    }
  }

  return ret;
}

void QRBLXBuild::interrupt()
{
  this->InterruptFlag.ref();
}

bool QRBLXBuild::interruptCallback()
{
#if QT_VERSION < QT_VERSION_CHECK(5, 14, 0)
  return this->InterruptFlag.load();
#else
  return this->InterruptFlag.loadRelaxed();
#endif
}

void QRBLXBuild::progressCallback(std::string const& msg, float percent)
{
  if (percent >= 0) {
    emit this->progressChanged(QString::fromStdString(msg), percent);
  } else {
    emit this->outputMessage(QString::fromStdString(msg));
  }
  QCoreApplication::processEvents();
}

void QRBLXBuild::messageCallback(std::string const& msg, char const* /*title*/)
{
  emit this->errorMessage(QString::fromStdString(msg));
  QCoreApplication::processEvents();
}

void QRBLXBuild::stdoutCallback(std::string const& msg)
{
  emit this->outputMessage(QString::fromStdString(msg));
  QCoreApplication::processEvents();
}

void QRBLXBuild::stderrCallback(std::string const& msg)
{
  emit this->outputMessage(QString::fromStdString(msg));
  QCoreApplication::processEvents();
}

void QRBLXBuild::setUpEnvironment() const
{
  auto env = QProcessEnvironment::systemEnvironment();
  for (auto const& key : env.keys()) {
    cmSystemTools::UnsetEnv(key.toStdString().c_str());
  }

  for (auto const& var : this->Environment.toStringList()) {
    cmSystemTools::PutEnv(var.toStdString());
  }
}

void QRBLXBuild::loadPresets()
{
  auto result = this->RBLXBuildPresetsGraph.ReadProjectPresets(
    this->SourceDirectory.toStdString(), "",
    cmRBLXBuildPresetsGraph::ReadOption::AllowNoFiles);
  if (!result) {
    emit this->presetLoadError(
      this->SourceDirectory,
      QString::fromStdString(
        this->RBLXBuildPresetsGraph.parseState.GetErrorMessage(false)));
  }

  QVector<QRBLXBuildPreset> presets;
  for (auto const& name : this->RBLXBuildPresetsGraph.ConfigurePresetOrder) {
    auto const& it = this->RBLXBuildPresetsGraph.ConfigurePresets[name];
    auto const& p = it.Unexpanded;
    if (p.Hidden) {
      continue;
    }

    QRBLXBuildPreset preset;
    preset.name = QString::fromStdString(p.Name);
    preset.displayName = QString::fromStdString(p.DisplayName);
    preset.description = QString::fromStdString(p.Description);
    preset.generator = QString::fromStdString(p.Generator);
    preset.architecture = QString::fromStdString(p.Architecture);
    preset.setArchitecture = !p.ArchitectureStrategy ||
      p.ArchitectureStrategy == cmRBLXBuildPresetsGraph::ArchToolsetStrategy::Set;
    preset.toolset = QString::fromStdString(p.Toolset);
    preset.setToolset = !p.ToolsetStrategy ||
      p.ToolsetStrategy == cmRBLXBuildPresetsGraph::ArchToolsetStrategy::Set;
    preset.enabled = it.Expanded && it.Expanded->ConditionResult &&
      ((p.OriginFile->Version >= 3 && p.Generator.empty()) ||
       std::find_if(this->AvailableGenerators.begin(),
                    this->AvailableGenerators.end(),
                    [&p](rblxbuild::GeneratorInfo const& g) {
                      return g.name == p.Generator;
                    }) != this->AvailableGenerators.end());
    presets.push_back(preset);
  }
  emit this->presetsChanged(presets);
}

QString QRBLXBuild::binaryDirectory() const
{
  return this->BinaryDirectory;
}

QString QRBLXBuild::relativeBinaryDirectory() const
{
  return this->MaybeRelativeBinaryDirectory;
}

QString QRBLXBuild::sourceDirectory() const
{
  return this->SourceDirectory;
}

QString QRBLXBuild::generator() const
{
  return this->Generator;
}

QProcessEnvironment QRBLXBuild::environment() const
{
  return this->Environment;
}

std::vector<rblxbuild::GeneratorInfo> const& QRBLXBuild::availableGenerators() const
{
  return AvailableGenerators;
}

void QRBLXBuild::deleteCache()
{
  // delete cache
  this->RBLXBuildInstance->DeleteCache(this->BinaryDirectory.toStdString());
  // reload to make our cache empty
  this->RBLXBuildInstance->LoadCache(this->BinaryDirectory.toStdString());
  // emit no generator and no properties
  this->setGenerator(QString());
  this->setToolset(QString());
  QRBLXBuildPropertyList props = this->properties();
  emit this->propertiesChanged(props);
}

void QRBLXBuild::reloadCache()
{
  // emit that the cache was cleaned out
  QRBLXBuildPropertyList props;
  emit this->propertiesChanged(props);
  // reload
  this->RBLXBuildInstance->LoadCache(this->BinaryDirectory.toStdString());
  // emit new cache properties
  props = this->properties();
  emit this->propertiesChanged(props);
}

void QRBLXBuild::setDebugOutput(bool flag)
{
  if (flag != this->RBLXBuildInstance->GetDebugOutput()) {
    this->RBLXBuildInstance->SetDebugOutputOn(flag);
    emit this->debugOutputChanged(flag);
  }
}

bool QRBLXBuild::getDebugOutput() const
{
  return this->RBLXBuildInstance->GetDebugOutput();
}

void QRBLXBuild::setDiagnosticAction(cmDiagnostics::DiagnosticCategory category,
                                 cmDiagnostics::DiagnosticAction action)
{
  this->RBLXBuildInstance->GetCurrentSnapshot().SetDiagnostic(category, action,
                                                          false);
}

cmDiagnosticAction QRBLXBuild::getDiagnosticAction(
  cmDiagnosticCategory category) const
{
  return this->RBLXBuildInstance->GetCurrentSnapshot().GetDiagnostic(category);
}

void QRBLXBuild::checkOpenPossible()
{
  std::string data = this->BinaryDirectory.toStdString();
  auto possible = this->RBLXBuildInstance->Open(data, rblxbuild::DryRun::Yes);
  emit openPossible(possible);
}
