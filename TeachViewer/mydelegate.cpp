#pragma execution_character_set("utf-8")
#include "mydelegate.h"
#include <QSpinBox>
#include <QComboBox>

MyDelegate::MyDelegate(QWidget *parent)
    : QStyledItemDelegate(parent)
{
}

QWidget *MyDelegate::createEditor(QWidget *parent,
    const QStyleOptionViewItem &/* option */,
    const QModelIndex & index ) const
{
    if(index.column() == 0) //只对第4列采用此方法编辑
    {
        QComboBox* box = new QComboBox(parent);
        box->addItems(QStringList()<<"优"<<"良"<<"差");
        return box;
    }
    return NULL;
}

void MyDelegate::setEditorData(QWidget *editor,
                                    const QModelIndex &index) const
{
    QString value = index.model()->data(index, Qt::EditRole).toString();
    QComboBox* box = static_cast<QComboBox*>(editor);
    box->setCurrentText(value);
}

void MyDelegate::setModelData(QWidget *editor, QAbstractItemModel *model,
                                   const QModelIndex &index) const
{
    QComboBox* box = static_cast<QComboBox*>(editor);
    model->setData(index, box->currentText(), Qt::EditRole);
}

void MyDelegate::updateEditorGeometry(QWidget *editor,
    const QStyleOptionViewItem &option, const QModelIndex &/* index */) const
{
    editor->setGeometry(option.rect);
}
