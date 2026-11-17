/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file LICENSE.rst or https://linearbuild.org/licensing for details.  */
#include "QLinearBuild.h"

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

#include "QLinearBuildSizeType.h"

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

QLinearBuild::QLinearBuild(QObject* p)
  : QObject(p)
  , StartEnvironment(QProcessEnvironment::systemEnvironment())
  , Environment(QProcessEnvironment::systemEnvironment())
{
  qRegisterMetaType<QLinearBuildProperty>();
  qRegisterMetaType<QLinearBuildPropertyList>();
  qRegisterMetaType<QProcessEnvironment>();
  qRegisterMetaType<QVector<QLinearBuildPreset>>();
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

  this->LinearBuildInstance = cm::make_unique<linearbuild>(cmState::Role::Project);
  this->LinearBuildInstance->SetLinearBuildEditCommand(
    cmSystemTools::GetLinearBuildGUICommand());
  this->LinearBuildInstance->SetProgressCallback(
    [this](std::string const& msg, float percent) {
      this->progressCallback(msg, percent);
    });

  cmSystemTools::SetInterruptCallback(
    [this] { return this->interruptCallback(); });

  std::vector<linearbuild::GeneratorInfo> generators;
  this->LinearBuildInstance->GetRegisteredGenerators(generators);

  for (linearbuild::GeneratorInfo const& gen : generators) {
    this->AvailableGenerators.push_back(gen);
  }
}

QLinearBuild::~QLinearBuild() = default;

void QLinearBuild::loadCache(QString const& dir)
{
  this->setBinaryDirectory(dir);
}

void QLinearBuild::setSourceDirectory(QString const& _dir)
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

void QLinearBuild::setBinaryDirectory(QString const& _dir)
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
    cmState* state = this->LinearBuildInstance->GetState();
    this->setGenerator(QString());
    this->setToolset(QString());
    this->setPlatform(QString());
    if (!this->LinearBuildInstance->LoadCache(this->BinaryDirectory.toStdString())) {
      QDir testDir(this->BinaryDirectory);
      if (testDir.exists("LinearBuildCache.txt")) {
        cmSystemTools::Error(
          "There is a LinearBuildCache.txt file for the current binary "
          "tree but linearbuild does not have permission to read it.  "
          "Please check the permissions of the directory you are trying to "
          "run LinearBuild on.");
      }
    }

    QLinearBuildPropertyList props = this->properties();
    if (!this->ApplyingPreset) {
      emit this->propertiesChanged(props);
    }
    cmValue homeDir = state->GetCacheEntryValue("LINEARBUILD_HOME_DIRECTORY");
    if (homeDir) {
      setSourceDirectory(QString(homeDir->c_str()));
    }
    cmValue gen = state->GetCacheEntryValue("LINEARBUILD_GENERATOR");
    if (gen) {
      cmValue extraGen =
        state->GetInitializedCacheValue("LINEARBUILD_EXTRA_GENERATOR");
      std::string curGen =
        cmExternalMakefileProjectGenerator::CreateFullGeneratorName(*gen,
                                                                    *extraGen);
      this->setGenerator(QString::fromStdString(curGen));
    }

    cmValue platform = state->GetCacheEntryValue("LINEARBUILD_GENERATOR_PLATFORM");
    if (platform) {
      this->setPlatform(QString(platform->c_str()));
    }

    cmValue toolset = state->GetCacheEntryValue("LINEARBUILD_GENERATOR_TOOLSET");
    if (toolset) {
      this->setToolset(QString(toolset->c_str()));
    }

    checkOpenPossible();
  }
}

void QLinearBuild::setPreset(QString const& name, bool setBinary)
{
  if (this->PresetName != name) {
    this->PresetName = name;
    if (!this->ApplyingPreset) {
      emit this->presetChanged(this->PresetName);
    }

    if (!name.isNull()) {
      std::string presetName(name.toStdString());
      auto const expandedPreset =
        this->LinearBuildPresetsGraph.ConfigurePresets[presetName].Expanded;
      if (expandedPreset) {
        if (setBinary && !expandedPreset->BinaryDir.empty()) {
          QString binaryDir =
            QString::fromStdString(expandedPreset->BinaryDir);
          this->setBinaryDirectory(binaryDir);
        }
        this->LinearBuildInstance->SetDiagnosticsFromPreset(expandedPreset->Warnings,
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

void QLinearBuild::applyPreset(QString const& name, bool setBinary,
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

void QLinearBuild::setGenerator(QString const& gen)
{
  if (this->Generator != gen) {
    this->Generator = gen;
    emit this->generatorChanged(this->Generator);
  }
}

void QLinearBuild::setPlatform(QString const& platform)
{
  if (this->Platform != platform) {
    this->Platform = platform;
    emit this->platformChanged(this->Platform);
  }
}

void QLinearBuild::setToolset(QString const& toolset)
{
  if (this->Toolset != toolset) {
    this->Toolset = toolset;
    emit this->toolsetChanged(this->Toolset);
  }
}

void QLinearBuild::setEnvironment(QProcessEnvironment const& environment)
{
  this->Environment = environment;
}

void QLinearBuild::configure()
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

    this->LinearBuildInstance->SetHomeDirectory(sanitizePath(this->SourceDirectory));
    this->LinearBuildInstance->SetHomeOutputDirectory(
      sanitizePath(this->BinaryDirectory));
    this->LinearBuildInstance->SetGlobalGenerator(
      this->LinearBuildInstance->CreateGlobalGenerator(
        this->Generator.toStdString()));
    this->LinearBuildInstance->SetGeneratorPlatform(this->Platform.toStdString());
    this->LinearBuildInstance->SetGeneratorToolset(this->Toolset.toStdString());
    this->LinearBuildInstance->LoadCache();
    this->LinearBuildInstance->PreLoadLinearBuildFiles();

    InterruptFlag = 0;
    cmSystemTools::ResetErrorOccurredFlag();

    err = this->LinearBuildInstance->Configure();

#ifdef Q_OS_WIN
    SetErrorMode(lastErrorMode);
#endif
  }

  emit this->propertiesChanged(this->properties());
  emit this->configureDone(err);
}

void QLinearBuild::generate()
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

    err = this->LinearBuildInstance->Generate();

#ifdef Q_OS_WIN
    SetErrorMode(lastErrorMode);
#endif
  }

  emit this->generateDone(err);
  checkOpenPossible();
}

void QLinearBuild::open()
{
#ifdef Q_OS_WIN
  UINT lastErrorMode = SetErrorMode(0);
#endif

  InterruptFlag = 0;
  cmSystemTools::ResetErrorOccurredFlag();

  auto successful = this->LinearBuildInstance->Open(
    this->BinaryDirectory.toStdString(), linearbuild::DryRun::No);

#ifdef Q_OS_WIN
  SetErrorMode(lastErrorMode);
#endif

  emit this->openDone(successful);
}

void QLinearBuild::setProperties(QLinearBuildPropertyList const& newProps)
{
  QLinearBuildPropertyList props = newProps;

  QStringList toremove;

  // set the value of properties
  cmState* state = this->LinearBuildInstance->GetState();
  std::vector<std::string> cacheKeys = state->GetCacheEntryKeys();
  for (std::string const& key : cacheKeys) {
    cmStateEnums::CacheEntryType t = state->GetCacheEntryType(key);
    if (t == cmStateEnums::INTERNAL || t == cmStateEnums::STATIC) {
      continue;
    }

    QLinearBuildProperty prop;
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
    this->LinearBuildInstance->UnwatchUnusedCli(s.toStdString());

    state->RemoveCacheEntry(s.toStdString());
  }

  // add some new properties
  foreach (QLinearBuildProperty const& s, props) {
    this->LinearBuildInstance->WatchUnusedCli(s.Key.toStdString());

    if (s.Type == QLinearBuildProperty::BOOL) {
      this->LinearBuildInstance->AddCacheEntry(
        s.Key.toStdString(), s.Value.toBool() ? "ON" : "OFF",
        s.Help.toStdString(), cmStateEnums::BOOL);
    } else if (s.Type == QLinearBuildProperty::STRING) {
      this->LinearBuildInstance->AddCacheEntry(
        s.Key.toStdString(), s.Value.toString().toStdString(),
        s.Help.toStdString(), cmStateEnums::STRING);
    } else if (s.Type == QLinearBuildProperty::PATH) {
      this->LinearBuildInstance->AddCacheEntry(
        s.Key.toStdString(), s.Value.toString().toStdString(),
        s.Help.toStdString(), cmStateEnums::PATH);
    } else if (s.Type == QLinearBuildProperty::FILEPATH) {
      this->LinearBuildInstance->AddCacheEntry(
        s.Key.toStdString(), s.Value.toString().toStdString(),
        s.Help.toStdString(), cmStateEnums::FILEPATH);
    }
  }

  this->LinearBuildInstance->SaveCache(this->BinaryDirectory.toStdString());
}

namespace {
template <typename T>
QLinearBuildProperty cache_to_property(T const& v)
{
  QLinearBuildProperty prop;
  prop.Key = QString::fromStdString(v.first);
  prop.Value = QString::fromStdString(v.second->Value);
  prop.Type = QLinearBuildProperty::STRING;
  if (!v.second->Type.empty()) {
    auto type = cmState::StringToCacheEntryType(v.second->Type);
    switch (type) {
      case cmStateEnums::BOOL:
        prop.Type = QLinearBuildProperty::BOOL;
        prop.Value = cmIsOn(v.second->Value);
        break;
      case cmStateEnums::PATH:
        prop.Type = QLinearBuildProperty::PATH;
        break;
      case cmStateEnums::FILEPATH:
        prop.Type = QLinearBuildProperty::FILEPATH;
        break;
      default:
        prop.Type = QLinearBuildProperty::STRING;
        break;
    }
  }
  return prop;
}

void add_to_property_list(QLinearBuildPropertyList& list, QLinearBuildProperty&& prop)
{
  // QLinearBuildCacheModel prefers variables earlier in the list rather than
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

QLinearBuildPropertyList QLinearBuild::properties() const
{
  QLinearBuildPropertyList ret;

  cmState* state = this->LinearBuildInstance->GetState();
  std::vector<std::string> cacheKeys = state->GetCacheEntryKeys();
  for (std::string const& key : cacheKeys) {
    cmStateEnums::CacheEntryType t = state->GetCacheEntryType(key);
    if (t == cmStateEnums::INTERNAL || t == cmStateEnums::STATIC ||
        t == cmStateEnums::UNINITIALIZED) {
      continue;
    }

    cmValue cachedValue = state->GetCacheEntryValue(key);

    QLinearBuildProperty prop;
    prop.Key = QString::fromStdString(key);
    if (cmValue hs = state->GetCacheEntryProperty(key, "HELPSTRING")) {
      prop.Help = QString(hs->c_str());
    }
    prop.Value = QString(cachedValue->c_str());
    prop.Advanced = state->GetCacheEntryPropertyAsBool(key, "ADVANCED");
    if (t == cmStateEnums::BOOL) {
      prop.Type = QLinearBuildProperty::BOOL;
      prop.Value = cmIsOn(*cachedValue);
    } else if (t == cmStateEnums::PATH) {
      prop.Type = QLinearBuildProperty::PATH;
    } else if (t == cmStateEnums::FILEPATH) {
      prop.Type = QLinearBuildProperty::FILEPATH;
    } else if (t == cmStateEnums::STRING) {
      prop.Type = QLinearBuildProperty::STRING;
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
      this->LinearBuildPresetsGraph.ConfigurePresets.at(presetName).Expanded;
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
        using CacheVariable = cmLinearBuildPresetsGraph::CacheVariable;
        CacheVariable var{ "FILEPATH", p->ToolchainFile };
        std::pair<std::string, cm::optional<CacheVariable>> value = {
          "LINEARBUILD_TOOLCHAIN_FILE", var
        };
        auto prop = cache_to_property(value);
        add_to_property_list(ret, std::move(prop));
      }
      if (!p->InstallDir.empty()) {
        using CacheVariable = cmLinearBuildPresetsGraph::CacheVariable;
        CacheVariable var{ "PATH", p->InstallDir };
        std::pair<std::string, cm::optional<CacheVariable>> value = {
          "LINEARBUILD_INSTALL_PREFIX", var
        };
        auto prop = cache_to_property(value);
        add_to_property_list(ret, std::move(prop));
      }
    }
  }

  return ret;
}

void QLinearBuild::interrupt()
{
  this->InterruptFlag.ref();
}

bool QLinearBuild::interruptCallback()
{
#if QT_VERSION < QT_VERSION_CHECK(5, 14, 0)
  return this->InterruptFlag.load();
#else
  return this->InterruptFlag.loadRelaxed();
#endif
}

void QLinearBuild::progressCallback(std::string const& msg, float percent)
{
  if (percent >= 0) {
    emit this->progressChanged(QString::fromStdString(msg), percent);
  } else {
    emit this->outputMessage(QString::fromStdString(msg));
  }
  QCoreApplication::processEvents();
}

void QLinearBuild::messageCallback(std::string const& msg, char const* /*title*/)
{
  emit this->errorMessage(QString::fromStdString(msg));
  QCoreApplication::processEvents();
}

void QLinearBuild::stdoutCallback(std::string const& msg)
{
  emit this->outputMessage(QString::fromStdString(msg));
  QCoreApplication::processEvents();
}

void QLinearBuild::stderrCallback(std::string const& msg)
{
  emit this->outputMessage(QString::fromStdString(msg));
  QCoreApplication::processEvents();
}

void QLinearBuild::setUpEnvironment() const
{
  auto env = QProcessEnvironment::systemEnvironment();
  for (auto const& key : env.keys()) {
    cmSystemTools::UnsetEnv(key.toStdString().c_str());
  }

  for (auto const& var : this->Environment.toStringList()) {
    cmSystemTools::PutEnv(var.toStdString());
  }
}

void QLinearBuild::loadPresets()
{
  auto result = this->LinearBuildPresetsGraph.ReadProjectPresets(
    this->SourceDirectory.toStdString(), "",
    cmLinearBuildPresetsGraph::ReadOption::AllowNoFiles);
  if (!result) {
    emit this->presetLoadError(
      this->SourceDirectory,
      QString::fromStdString(
        this->LinearBuildPresetsGraph.parseState.GetErrorMessage(false)));
  }

  QVector<QLinearBuildPreset> presets;
  for (auto const& name : this->LinearBuildPresetsGraph.ConfigurePresetOrder) {
    auto const& it = this->LinearBuildPresetsGraph.ConfigurePresets[name];
    auto const& p = it.Unexpanded;
    if (p.Hidden) {
      continue;
    }

    QLinearBuildPreset preset;
    preset.name = QString::fromStdString(p.Name);
    preset.displayName = QString::fromStdString(p.DisplayName);
    preset.description = QString::fromStdString(p.Description);
    preset.generator = QString::fromStdString(p.Generator);
    preset.architecture = QString::fromStdString(p.Architecture);
    preset.setArchitecture = !p.ArchitectureStrategy ||
      p.ArchitectureStrategy == cmLinearBuildPresetsGraph::ArchToolsetStrategy::Set;
    preset.toolset = QString::fromStdString(p.Toolset);
    preset.setToolset = !p.ToolsetStrategy ||
      p.ToolsetStrategy == cmLinearBuildPresetsGraph::ArchToolsetStrategy::Set;
    preset.enabled = it.Expanded && it.Expanded->ConditionResult &&
      ((p.OriginFile->Version >= 3 && p.Generator.empty()) ||
       std::find_if(this->AvailableGenerators.begin(),
                    this->AvailableGenerators.end(),
                    [&p](linearbuild::GeneratorInfo const& g) {
                      return g.name == p.Generator;
                    }) != this->AvailableGenerators.end());
    presets.push_back(preset);
  }
  emit this->presetsChanged(presets);
}

QString QLinearBuild::binaryDirectory() const
{
  return this->BinaryDirectory;
}

QString QLinearBuild::relativeBinaryDirectory() const
{
  return this->MaybeRelativeBinaryDirectory;
}

QString QLinearBuild::sourceDirectory() const
{
  return this->SourceDirectory;
}

QString QLinearBuild::generator() const
{
  return this->Generator;
}

QProcessEnvironment QLinearBuild::environment() const
{
  return this->Environment;
}

std::vector<linearbuild::GeneratorInfo> const& QLinearBuild::availableGenerators() const
{
  return AvailableGenerators;
}

void QLinearBuild::deleteCache()
{
  // delete cache
  this->LinearBuildInstance->DeleteCache(this->BinaryDirectory.toStdString());
  // reload to make our cache empty
  this->LinearBuildInstance->LoadCache(this->BinaryDirectory.toStdString());
  // emit no generator and no properties
  this->setGenerator(QString());
  this->setToolset(QString());
  QLinearBuildPropertyList props = this->properties();
  emit this->propertiesChanged(props);
}

void QLinearBuild::reloadCache()
{
  // emit that the cache was cleaned out
  QLinearBuildPropertyList props;
  emit this->propertiesChanged(props);
  // reload
  this->LinearBuildInstance->LoadCache(this->BinaryDirectory.toStdString());
  // emit new cache properties
  props = this->properties();
  emit this->propertiesChanged(props);
}

void QLinearBuild::setDebugOutput(bool flag)
{
  if (flag != this->LinearBuildInstance->GetDebugOutput()) {
    this->LinearBuildInstance->SetDebugOutputOn(flag);
    emit this->debugOutputChanged(flag);
  }
}

bool QLinearBuild::getDebugOutput() const
{
  return this->LinearBuildInstance->GetDebugOutput();
}

void QLinearBuild::setDiagnosticAction(cmDiagnostics::DiagnosticCategory category,
                                 cmDiagnostics::DiagnosticAction action)
{
  this->LinearBuildInstance->GetCurrentSnapshot().SetDiagnostic(category, action,
                                                          false);
}

cmDiagnosticAction QLinearBuild::getDiagnosticAction(
  cmDiagnosticCategory category) const
{
  return this->LinearBuildInstance->GetCurrentSnapshot().GetDiagnostic(category);
}

void QLinearBuild::checkOpenPossible()
{
  std::string data = this->BinaryDirectory.toStdString();
  auto possible = this->LinearBuildInstance->Open(data, linearbuild::DryRun::Yes);
  emit openPossible(possible);
}
