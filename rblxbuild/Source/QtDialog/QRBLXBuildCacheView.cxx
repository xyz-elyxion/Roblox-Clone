/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file LICENSE.rst or https://rblxbuild.org/licensing for details.  */
#include "QRBLXBuildCacheView.h"

#include <QApplication>
#include <QEvent>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QKeyEvent>
#include <QMetaProperty>
#include <QSortFilterProxyModel>
#include <QStyle>

#include "QRBLXBuildSizeType.h"
#include "QRBLXBuildWidgets.h"

// filter for searches
class QRBLXBuildSearchFilter : public QSortFilterProxyModel
{
public:
  QRBLXBuildSearchFilter(QObject* o)
    : QSortFilterProxyModel(o)
  {
  }

protected:
  bool filterAcceptsRow(int row, QModelIndex const& p) const override
  {
    QStringList strs;
    QAbstractItemModel const* m = this->sourceModel();
    QModelIndex idx = m->index(row, 0, p);

    // if there are no children, get strings for column 0 and 1
    if (!m->hasChildren(idx)) {
      strs.append(m->data(idx).toString());
      idx = m->index(row, 1, p);
      strs.append(m->data(idx).toString());
    } else {
      // get strings for children entries to compare with
      // instead of comparing with the parent
      int num = m->rowCount(idx);
      for (int i = 0; i < num; i++) {
        QModelIndex tmpidx = m->index(i, 0, idx);
        strs.append(m->data(tmpidx).toString());
        tmpidx = m->index(i, 1, idx);
        strs.append(m->data(tmpidx).toString());
      }
    }

    // check all strings for a match
    foreach (QString const& str, strs) {
#if (QT_VERSION >= QT_VERSION_CHECK(5, 15, 0))
      if (str.contains(this->filterRegularExpression())) {
#else
      if (str.contains(this->filterRegExp())) {
#endif
        return true;
      }
    }

    return false;
  }
};

// filter for searches
class QRBLXBuildAdvancedFilter : public QSortFilterProxyModel
{
public:
  QRBLXBuildAdvancedFilter(QObject* o)
    : QSortFilterProxyModel(o)
  {
  }

  void setShowAdvanced(bool f)
  {
    this->ShowAdvanced = f;
    this->invalidate();
  }
  bool showAdvanced() const { return this->ShowAdvanced; }

protected:
  bool ShowAdvanced = false;

  bool filterAcceptsRow(int row, QModelIndex const& p) const override
  {
    QAbstractItemModel const* m = this->sourceModel();
    QModelIndex idx = m->index(row, 0, p);

    // if there are no children
    if (!m->hasChildren(idx)) {
      bool adv = m->data(idx, QRBLXBuildCacheModel::AdvancedRole).toBool();
      return !adv || this->ShowAdvanced;
    }

    // check children
    int num = m->rowCount(idx);
    for (int i = 0; i < num; i++) {
      bool accept = this->filterAcceptsRow(i, idx);
      if (accept) {
        return true;
      }
    }
    return false;
  }
};

QRBLXBuildCacheView::QRBLXBuildCacheView(QWidget* p)
  : QTreeView(p)
{
  // hook up our model and search/filter proxies
  this->CacheModel = new QRBLXBuildCacheModel(this);
  this->AdvancedFilter = new QRBLXBuildAdvancedFilter(this);
  this->AdvancedFilter->setSourceModel(this->CacheModel);
  this->AdvancedFilter->setDynamicSortFilter(true);
  this->SearchFilter = new QRBLXBuildSearchFilter(this);
  this->SearchFilter->setSourceModel(this->AdvancedFilter);
  this->SearchFilter->setFilterCaseSensitivity(Qt::CaseInsensitive);
  this->SearchFilter->setDynamicSortFilter(true);
  this->setModel(this->SearchFilter);

  // our delegate for creating our editors
  QRBLXBuildCacheModelDelegate* delegate = new QRBLXBuildCacheModelDelegate(this);
  this->setItemDelegate(delegate);

  this->setUniformRowHeights(true);

  this->setEditTriggers(QAbstractItemView::AllEditTriggers);

  // tab, backtab doesn't step through items
  this->setTabKeyNavigation(false);

  this->setRootIsDecorated(false);
}

bool QRBLXBuildCacheView::event(QEvent* e)
{
  if (e->type() == QEvent::Show) {
    this->header()->setDefaultSectionSize(this->viewport()->width() / 2);
  }
  return QTreeView::event(e);
}

QRBLXBuildCacheModel* QRBLXBuildCacheView::cacheModel() const
{
  return this->CacheModel;
}

QModelIndex QRBLXBuildCacheView::moveCursor(CursorAction act,
                                        Qt::KeyboardModifiers mod)
{
  // want home/end to go to begin/end of rows, not columns
  if (act == MoveHome) {
    return this->model()->index(0, 1);
  }
  if (act == MoveEnd) {
    return this->model()->index(this->model()->rowCount() - 1, 1);
  }
  return QTreeView::moveCursor(act, mod);
}

void QRBLXBuildCacheView::setShowAdvanced(bool s)
{
  this->SearchFilter->invalidate();
  this->AdvancedFilter->setShowAdvanced(s);
}

bool QRBLXBuildCacheView::showAdvanced() const
{
  return this->AdvancedFilter->showAdvanced();
}

bool QRBLXBuildCacheView::setSearchFilter(QString const& s)
{
  return QtRBLXBuild::setSearchFilter(this->SearchFilter, s);
}

QRBLXBuildCacheModel::QRBLXBuildCacheModel(QObject* p)
  : QStandardItemModel(p)
  , EditEnabled(true)
  , NewPropertyCount(0)
  , View(FlatView)
{
  this->ShowNewProperties = true;
  QStringList labels;
  labels << tr("Name") << tr("Value");
  this->setHorizontalHeaderLabels(labels);
}

QRBLXBuildCacheModel::~QRBLXBuildCacheModel() = default;

static size_t qHash(QRBLXBuildProperty const& p)
{
  return qHash(p.Key);
}

void QRBLXBuildCacheModel::setShowNewProperties(bool f)
{
  this->ShowNewProperties = f;
}

void QRBLXBuildCacheModel::clear()
{
  this->QStandardItemModel::clear();
  this->NewPropertyCount = 0;

  QStringList labels;
  labels << tr("Name") << tr("Value");
  this->setHorizontalHeaderLabels(labels);
}

void QRBLXBuildCacheModel::setProperties(QRBLXBuildPropertyList const& props)
{
  this->beginResetModel();

  QSet<QRBLXBuildProperty> newProps;
  QSet<QRBLXBuildProperty> newProps2;

  if (this->ShowNewProperties) {
#if QT_VERSION < QT_VERSION_CHECK(5, 14, 0)
    newProps = props.toSet();
#else
    newProps = QSet<QRBLXBuildProperty>(props.begin(), props.end());
#endif
    newProps2 = newProps;
#if QT_VERSION < QT_VERSION_CHECK(5, 14, 0)
    QSet<QRBLXBuildProperty> oldProps = this->properties().toSet();
#else
    QRBLXBuildPropertyList const& oldPropsList = this->properties();
    QSet<QRBLXBuildProperty> oldProps =
      QSet<QRBLXBuildProperty>(oldPropsList.begin(), oldPropsList.end());
#endif
    oldProps.intersect(newProps);
    newProps.subtract(oldProps);
    newProps2.subtract(newProps);
  } else {
#if QT_VERSION < QT_VERSION_CHECK(5, 14, 0)
    newProps2 = props.toSet();
#else
    newProps2 = QSet<QRBLXBuildProperty>(props.begin(), props.end());
#endif
  }

  bool b = this->blockSignals(true);

  // Empty the model. Avoid QStandardItemModel::clear(), because that calls
  // beginResetModel() internally, and we can't nest such calls.
  this->removeRows(0, this->rowCount());

  this->NewPropertyCount = newProps.size();

  if (View == FlatView) {
    QRBLXBuildPropertyList newP = newProps.values();
    QRBLXBuildPropertyList newP2 = newProps2.values();
#if QT_VERSION >= QT_VERSION_CHECK(5, 9, 0)
    std::sort(newP.begin(), newP.end());
    std::sort(newP2.begin(), newP2.end());
#else
    qSort(newP);
    qSort(newP2);
#endif
    int row_count = 0;
    foreach (QRBLXBuildProperty const& p, newP) {
      this->insertRow(row_count);
      this->setPropertyData(this->index(row_count, 0), p, true);
      row_count++;
    }
    foreach (QRBLXBuildProperty const& p, newP2) {
      this->insertRow(row_count);
      this->setPropertyData(this->index(row_count, 0), p, false);
      row_count++;
    }
  } else if (this->View == GroupView) {
    QMap<QString, QRBLXBuildPropertyList> newPropsTree;
    QRBLXBuildCacheModel::breakProperties(newProps, newPropsTree);
    QMap<QString, QRBLXBuildPropertyList> newPropsTree2;
    QRBLXBuildCacheModel::breakProperties(newProps2, newPropsTree2);

    QStandardItem* root = this->invisibleRootItem();

    for (QMap<QString, QRBLXBuildPropertyList>::const_iterator iter =
           newPropsTree.begin();
         iter != newPropsTree.end(); ++iter) {
      QString const& key = iter.key();
      QRBLXBuildPropertyList const& props2 = iter.value();

      QList<QStandardItem*> parentItems;
      parentItems.append(
        new QStandardItem(key.isEmpty() ? tr("Ungrouped Entries") : key));
      parentItems.append(new QStandardItem());
#if QT_VERSION >= QT_VERSION_CHECK(5, 9, 0)
      parentItems[0]->setData(QBrush(QColor(255, 100, 100)),
                              Qt::BackgroundRole);
      parentItems[1]->setData(QBrush(QColor(255, 100, 100)),
                              Qt::BackgroundRole);
#else
      parentItems[0]->setData(QBrush(QColor(255, 100, 100)),
                              Qt::BackgroundColorRole);
      parentItems[1]->setData(QBrush(QColor(255, 100, 100)),
                              Qt::BackgroundColorRole);
#endif
      parentItems[0]->setData(1, GroupRole);
      parentItems[1]->setData(1, GroupRole);
      root->appendRow(parentItems);

      cm_qsizetype num = props2.size();
      for (cm_qsizetype i = 0; i < num; i++) {
        QRBLXBuildProperty const& prop = props2[i];
        QList<QStandardItem*> items;
        items.append(new QStandardItem());
        items.append(new QStandardItem());
        parentItems[0]->appendRow(items);
        this->setPropertyData(this->indexFromItem(items[0]), prop, true);
      }
    }

    for (QMap<QString, QRBLXBuildPropertyList>::const_iterator iter =
           newPropsTree2.begin();
         iter != newPropsTree2.end(); ++iter) {
      QString const& key = iter.key();
      QRBLXBuildPropertyList const& props2 = iter.value();

      QStandardItem* parentItem =
        new QStandardItem(key.isEmpty() ? tr("Ungrouped Entries") : key);
      root->appendRow(parentItem);
      parentItem->setData(1, GroupRole);

      cm_qsizetype num = props2.size();
      for (cm_qsizetype i = 0; i < num; i++) {
        QRBLXBuildProperty const& prop = props2[i];
        QList<QStandardItem*> items;
        items.append(new QStandardItem());
        items.append(new QStandardItem());
        parentItem->appendRow(items);
        this->setPropertyData(this->indexFromItem(items[0]), prop, false);
      }
    }
  }

  this->blockSignals(b);
  this->endResetModel();
}

QRBLXBuildCacheModel::ViewType QRBLXBuildCacheModel::viewType() const
{
  return this->View;
}

void QRBLXBuildCacheModel::setViewType(QRBLXBuildCacheModel::ViewType t)
{
  this->View = t;

  QRBLXBuildPropertyList props = this->properties();
  QRBLXBuildPropertyList oldProps;
  cm_qsizetype numNew = this->NewPropertyCount;
  cm_qsizetype numTotal = props.count();
  for (cm_qsizetype i = numNew; i < numTotal; i++) {
    oldProps.append(props[i]);
  }

  bool b = this->blockSignals(true);
  this->clear();
  this->setProperties(oldProps);
  this->setProperties(props);
  this->blockSignals(b);
}

void QRBLXBuildCacheModel::setPropertyData(QModelIndex const& idx1,
                                       QRBLXBuildProperty const& prop, bool isNew)
{
  QModelIndex idx2 = idx1.sibling(idx1.row(), 1);

  this->setData(idx1, prop.Key, Qt::DisplayRole);
  this->setData(idx1, prop.Help, QRBLXBuildCacheModel::HelpRole);
  this->setData(idx1, prop.Type, QRBLXBuildCacheModel::TypeRole);
  this->setData(idx1, prop.Advanced, QRBLXBuildCacheModel::AdvancedRole);

  if (prop.Type == QRBLXBuildProperty::BOOL) {
    int check = prop.Value.toBool() ? Qt::Checked : Qt::Unchecked;
    this->setData(idx2, check, Qt::CheckStateRole);
  } else {
    this->setData(idx2, prop.Value, Qt::DisplayRole);
  }
  this->setData(idx2, prop.Help, QRBLXBuildCacheModel::HelpRole);

  if (!prop.Strings.isEmpty()) {
    this->setData(idx1, prop.Strings, QRBLXBuildCacheModel::StringsRole);
  }

  if (isNew) {
#if QT_VERSION >= QT_VERSION_CHECK(5, 9, 0)
    this->setData(idx1, QBrush(QColor(255, 100, 100)), Qt::BackgroundRole);
    this->setData(idx2, QBrush(QColor(255, 100, 100)), Qt::BackgroundRole);
#else
    this->setData(idx1, QBrush(QColor(255, 100, 100)),
                  Qt::BackgroundColorRole);
    this->setData(idx2, QBrush(QColor(255, 100, 100)),
                  Qt::BackgroundColorRole);
#endif
  }
}

void QRBLXBuildCacheModel::getPropertyData(QModelIndex const& idx1,
                                       QRBLXBuildProperty& prop) const
{
  QModelIndex idx2 = idx1.sibling(idx1.row(), 1);

  prop.Key = this->data(idx1, Qt::DisplayRole).toString();
  prop.Help = this->data(idx1, HelpRole).toString();
  prop.Type = static_cast<QRBLXBuildProperty::PropertyType>(
    this->data(idx1, TypeRole).toInt());
  prop.Advanced = this->data(idx1, AdvancedRole).toBool();
  prop.Strings =
    this->data(idx1, QRBLXBuildCacheModel::StringsRole).toStringList();
  if (prop.Type == QRBLXBuildProperty::BOOL) {
    int check = this->data(idx2, Qt::CheckStateRole).toInt();
    prop.Value = check == Qt::Checked;
  } else {
    prop.Value = this->data(idx2, Qt::DisplayRole).toString();
  }
}

QString QRBLXBuildCacheModel::prefix(QString const& s)
{
  QString prefix = s.section('_', 0, 0);
  if (prefix == s) {
    prefix = QString();
  }
  return prefix;
}

void QRBLXBuildCacheModel::breakProperties(
  QSet<QRBLXBuildProperty> const& props, QMap<QString, QRBLXBuildPropertyList>& result)
{
  QMap<QString, QRBLXBuildPropertyList> tmp;
  // return a map of properties grouped by prefixes, and sorted
  foreach (QRBLXBuildProperty const& p, props) {
    QString prefix = QRBLXBuildCacheModel::prefix(p.Key);
    tmp[prefix].append(p);
  }
  // sort it and re-org any properties with only one sub item
  QRBLXBuildPropertyList reorgProps;
  QMap<QString, QRBLXBuildPropertyList>::iterator iter;
  for (iter = tmp.begin(); iter != tmp.end();) {
    if (iter->count() == 1) {
      reorgProps.append((*iter)[0]);
      iter = tmp.erase(iter);
    } else {
#if QT_VERSION >= QT_VERSION_CHECK(5, 9, 0)
      std::sort(iter->begin(), iter->end());
#else
      qSort(*iter);
#endif
      ++iter;
    }
  }
  if (reorgProps.count()) {
    tmp[QString()] += reorgProps;
  }
  result = tmp;
}

QRBLXBuildPropertyList QRBLXBuildCacheModel::properties() const
{
  QRBLXBuildPropertyList props;

  if (!this->rowCount()) {
    return props;
  }

  QVector<QModelIndex> idxs;
  idxs.append(this->index(0, 0));

  // walk the entire model for property entries
  // this works regardless of a flat view or a tree view
  while (!idxs.isEmpty()) {
    QModelIndex idx = idxs.last();
    if (this->hasChildren(idx) && this->rowCount(idx)) {
      idxs.append(this->index(0, 0, idx));
    } else {
      if (!data(idx, GroupRole).toInt()) {
        // get data
        QRBLXBuildProperty prop;
        this->getPropertyData(idx, prop);
        props.append(prop);
      }

      // go to the next in the tree
      while (!idxs.isEmpty() &&
             (
#if QT_VERSION < QT_VERSION_CHECK(5, 1, 0)
               (idxs.last().row() + 1) >= rowCount(idxs.last().parent()) ||
#endif
               !idxs.last().sibling(idxs.last().row() + 1, 0).isValid())) {
        idxs.remove(idxs.size() - 1);
      }
      if (!idxs.isEmpty()) {
        idxs.last() = idxs.last().sibling(idxs.last().row() + 1, 0);
      }
    }
  }

  return props;
}

bool QRBLXBuildCacheModel::insertProperty(QRBLXBuildProperty::PropertyType t,
                                      QString const& name,
                                      QString const& description,
                                      QVariant const& value, bool advanced)
{
  QRBLXBuildProperty prop;
  prop.Key = name;
  prop.Value = value;
  prop.Help = description;
  prop.Type = t;
  prop.Advanced = advanced;

  // insert at beginning
  this->insertRow(0);
  this->setPropertyData(this->index(0, 0), prop, true);
  this->NewPropertyCount++;
  return true;
}

void QRBLXBuildCacheModel::setEditEnabled(bool e)
{
  this->EditEnabled = e;
}

bool QRBLXBuildCacheModel::editEnabled() const
{
  return this->EditEnabled;
}

cm_qsizetype QRBLXBuildCacheModel::newPropertyCount() const
{
  return this->NewPropertyCount;
}

Qt::ItemFlags QRBLXBuildCacheModel::flags(QModelIndex const& idx) const
{
  Qt::ItemFlags f = QStandardItemModel::flags(idx);
  if (!this->EditEnabled) {
    f &= ~Qt::ItemIsEditable;
    return f;
  }
  if (QRBLXBuildProperty::BOOL == this->data(idx, TypeRole).toInt()) {
    f |= Qt::ItemIsUserCheckable;
  }
  return f;
}

QModelIndex QRBLXBuildCacheModel::buddy(QModelIndex const& idx) const
{
  if (!this->hasChildren(idx) &&
      this->data(idx, TypeRole).toInt() != QRBLXBuildProperty::BOOL) {
    return this->index(idx.row(), 1, idx.parent());
  }
  return idx;
}

QRBLXBuildCacheModelDelegate::QRBLXBuildCacheModelDelegate(QObject* p)
  : QItemDelegate(p)
  , FileDialogFlag(false)
{
}

void QRBLXBuildCacheModelDelegate::setFileDialogFlag(bool f)
{
  this->FileDialogFlag = f;
}

QWidget* QRBLXBuildCacheModelDelegate::createEditor(
  QWidget* p, QStyleOptionViewItem const& /*option*/,
  QModelIndex const& idx) const
{
  QModelIndex var = idx.sibling(idx.row(), 0);
  int type = var.data(QRBLXBuildCacheModel::TypeRole).toInt();
  if (type == QRBLXBuildProperty::BOOL) {
    return nullptr;
  }
  if (type == QRBLXBuildProperty::PATH) {
    QRBLXBuildPathEditor* editor =
      new QRBLXBuildPathEditor(p, var.data(Qt::DisplayRole).toString());
    QObject::connect(editor, &QRBLXBuildPathEditor::fileDialogExists, this,
                     &QRBLXBuildCacheModelDelegate::setFileDialogFlag);
    return editor;
  }
  if (type == QRBLXBuildProperty::FILEPATH) {
    QRBLXBuildFilePathEditor* editor =
      new QRBLXBuildFilePathEditor(p, var.data(Qt::DisplayRole).toString());
    QObject::connect(editor, &QRBLXBuildPathEditor::fileDialogExists, this,
                     &QRBLXBuildCacheModelDelegate::setFileDialogFlag);
    return editor;
  }
  if (type == QRBLXBuildProperty::STRING &&
      var.data(QRBLXBuildCacheModel::StringsRole).isValid()) {
    QRBLXBuildComboBox* editor = new QRBLXBuildComboBox(
      p, var.data(QRBLXBuildCacheModel::StringsRole).toStringList());
    editor->setFrame(false);
    return editor;
  }

  QLineEdit* editor = new QLineEdit(p);
  editor->setFrame(false);
  return editor;
}

bool QRBLXBuildCacheModelDelegate::editorEvent(QEvent* e,
                                           QAbstractItemModel* model,
                                           QStyleOptionViewItem const& option,
                                           QModelIndex const& index)
{
  Qt::ItemFlags flags = model->flags(index);
  if (!(flags & Qt::ItemIsUserCheckable) ||
      !(option.state & QStyle::State_Enabled) ||
      !(flags & Qt::ItemIsEnabled)) {
    return false;
  }

  QVariant value = index.data(Qt::CheckStateRole);
  if (!value.isValid()) {
    return false;
  }

  if ((e->type() == QEvent::MouseButtonRelease) ||
      (e->type() == QEvent::MouseButtonDblClick)) {
    // eat the double click events inside the check rect
    if (e->type() == QEvent::MouseButtonDblClick) {
      return true;
    }
  } else if (e->type() == QEvent::KeyPress) {
    if (static_cast<QKeyEvent*>(e)->key() != Qt::Key_Space &&
        static_cast<QKeyEvent*>(e)->key() != Qt::Key_Select) {
      return false;
    }
  } else {
    return false;
  }

  Qt::CheckState state =
    (static_cast<Qt::CheckState>(value.toInt()) == Qt::Checked ? Qt::Unchecked
                                                               : Qt::Checked);
  bool success = model->setData(index, state, Qt::CheckStateRole);
  if (success) {
    this->recordChange(model, index);
  }
  return success;
}

bool QRBLXBuildCacheModelDelegate::eventFilter(QObject* object, QEvent* evt)
{
  // FIXME: This filter avoids a crash when opening a file dialog
  // with the '...' button on a cache entry line in the GUI.
  // Previously this filter was commented as a workaround for Qt issue 205903,
  // but that was fixed in Qt 4.5.0 and the crash still occurs as of Qt 5.14
  // without this filter.  This needs further investigation.
  if (evt->type() == QEvent::FocusOut && this->FileDialogFlag) {
    return false;
  }
  return QItemDelegate::eventFilter(object, evt);
}

void QRBLXBuildCacheModelDelegate::setModelData(QWidget* editor,
                                            QAbstractItemModel* model,
                                            QModelIndex const& index) const
{
  QItemDelegate::setModelData(editor, model, index);
  const_cast<QRBLXBuildCacheModelDelegate*>(this)->recordChange(model, index);
}

QSize QRBLXBuildCacheModelDelegate::sizeHint(QStyleOptionViewItem const& option,
                                         QModelIndex const& index) const
{
  QSize sz = QItemDelegate::sizeHint(option, index);
  QStyle* style = QApplication::style();

  // increase to checkbox size
  QStyleOptionButton opt;
  opt.QStyleOption::operator=(option);
#if QT_VERSION >= QT_VERSION_CHECK(5, 13, 0)
  sz = sz.expandedTo(
    style->subElementRect(QStyle::SE_ItemViewItemCheckIndicator, &opt, nullptr)
      .size());
#else
  sz = sz.expandedTo(
    style->subElementRect(QStyle::SE_ViewItemCheckIndicator, &opt, nullptr)
      .size());
#endif

  return sz;
}

QSet<QRBLXBuildProperty> QRBLXBuildCacheModelDelegate::changes() const
{
  return mChanges;
}

void QRBLXBuildCacheModelDelegate::clearChanges()
{
  mChanges.clear();
}

void QRBLXBuildCacheModelDelegate::recordChange(QAbstractItemModel* model,
                                            QModelIndex const& index)
{
  QModelIndex idx = index;
  QAbstractItemModel* mymodel = model;
  while (qobject_cast<QAbstractProxyModel*>(mymodel)) {
    idx = static_cast<QAbstractProxyModel*>(mymodel)->mapToSource(idx);
    mymodel = static_cast<QAbstractProxyModel*>(mymodel)->sourceModel();
  }
  QRBLXBuildCacheModel* cache_model = qobject_cast<QRBLXBuildCacheModel*>(mymodel);
  if (cache_model && idx.isValid()) {
    QRBLXBuildProperty prop;
    idx = idx.sibling(idx.row(), 0);
    cache_model->getPropertyData(idx, prop);

    // clean out an old one
    QSet<QRBLXBuildProperty>::iterator iter = mChanges.find(prop);
    if (iter != mChanges.end()) {
      mChanges.erase(iter);
    }
    // now add the new item
    mChanges.insert(prop);
  }
}
