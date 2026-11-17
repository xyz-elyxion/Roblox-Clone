/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file LICENSE.rst or https://linearbuild.org/licensing for details.  */
#pragma once

#include <QItemDelegate>
#include <QSet>
#include <QStandardItemModel>
#include <QTreeView>

#include "QLinearBuild.h"
#include "QLinearBuildSizeType.h"

class QSortFilterProxyModel;
class QLinearBuildCacheModel;
class QLinearBuildAdvancedFilter;

/// Qt view class for cache properties
class QLinearBuildCacheView : public QTreeView
{
  Q_OBJECT
public:
  QLinearBuildCacheView(QWidget* p);

  // retrieve the QLinearBuildCacheModel storing all the pointers
  // this isn't necessarily the model one would get from model()
  QLinearBuildCacheModel* cacheModel() const;

  // get whether to show advanced entries
  bool showAdvanced() const;

  QSize sizeHint() const { return QSize(200, 200); }

  // set the search filter string.  any property key or value not matching will
  // be filtered out
  bool setSearchFilter(QString const&);

public slots:
  // set whether to show advanced entries
  void setShowAdvanced(bool);

protected:
  QModelIndex moveCursor(CursorAction, Qt::KeyboardModifiers);
  bool event(QEvent* e);
  QLinearBuildCacheModel* CacheModel;
  QLinearBuildAdvancedFilter* AdvancedFilter;
  QSortFilterProxyModel* SearchFilter;
};

/// Qt model class for cache properties
class QLinearBuildCacheModel : public QStandardItemModel
{
  Q_OBJECT
public:
  QLinearBuildCacheModel(QObject* parent = nullptr);
  ~QLinearBuildCacheModel();

  // roles used to retrieve extra data such has help strings, types of
  // properties, and the advanced flag
  enum
  {
    HelpRole = Qt::ToolTipRole,
    TypeRole = Qt::UserRole,
    AdvancedRole,
    StringsRole,
    GroupRole
  };

  enum ViewType
  {
    FlatView,
    GroupView
  };

public slots:
  // set a list of properties.  This list will be sorted and grouped according
  // to prefix.  Any property that existed already and which is found in this
  // list of properties to set will become an old property.  All others will
  // become new properties and be marked red.
  void setProperties(QLinearBuildPropertyList const& props);

  // set whether to show new properties in red
  void setShowNewProperties(bool);

  // clear everything from the model
  void clear();

  // set flag whether the model can currently be edited.
  void setEditEnabled(bool);

  // insert a new property at a row specifying all the information about the
  // property
  bool insertProperty(QLinearBuildProperty::PropertyType t, QString const& name,
                      QString const& description, QVariant const& value,
                      bool advanced);

public:
  // get the properties
  QLinearBuildPropertyList properties() const;

  // editing enabled
  bool editEnabled() const;

  // returns how many new properties there are
  cm_qsizetype newPropertyCount() const;

  // return flags (overloaded to modify flag based on EditEnabled flag)
  Qt::ItemFlags flags(QModelIndex const& index) const;
  QModelIndex buddy(QModelIndex const& idx) const;

  // get the data in the model for this property
  void getPropertyData(QModelIndex const& idx1, QLinearBuildProperty& prop) const;

  // set the view type
  void setViewType(ViewType t);
  ViewType viewType() const;

protected:
  bool EditEnabled;
  cm_qsizetype NewPropertyCount;
  bool ShowNewProperties;
  ViewType View;

  // set the data in the model for this property
  void setPropertyData(QModelIndex const& idx1, QLinearBuildProperty const& p,
                       bool isNew);

  // breaks up he property list into groups
  // where each group has the same prefix up to the first underscore
  static void breakProperties(QSet<QLinearBuildProperty> const& props,
                              QMap<QString, QLinearBuildPropertyList>& result);

  // gets the prefix of a string up to the first _
  static QString prefix(QString const& s);
};

/// Qt delegate class for interaction (or other customization)
/// with cache properties
class QLinearBuildCacheModelDelegate : public QItemDelegate
{
  Q_OBJECT
public:
  QLinearBuildCacheModelDelegate(QObject* p);
  /// create our own editors for cache properties
  QWidget* createEditor(QWidget* parent, QStyleOptionViewItem const& option,
                        QModelIndex const& index) const;
  bool editorEvent(QEvent* event, QAbstractItemModel* model,
                   QStyleOptionViewItem const& option,
                   QModelIndex const& index);
  bool eventFilter(QObject* object, QEvent* event);
  void setModelData(QWidget* editor, QAbstractItemModel* model,
                    QModelIndex const& index) const;
  QSize sizeHint(QStyleOptionViewItem const& option,
                 QModelIndex const& index) const;

  QSet<QLinearBuildProperty> changes() const;
  void clearChanges();

protected slots:
  void setFileDialogFlag(bool);

protected:
  bool FileDialogFlag;
  // record a change to an item in the model.
  // this simply saves the item in the set of changes
  void recordChange(QAbstractItemModel* model, QModelIndex const& index);

  // properties changed by user via this delegate
  QSet<QLinearBuildProperty> mChanges;
};
