#ifndef MYDELEGATE_H
#define MYDELEGATE_H

#include <QObject>
#include<QStyledItemDelegate>

class MyDelegate:public QStyledItemDelegate
{
    Q_OBJECT
public:
    MyDelegate(QWidget *parent = nullptr);
    QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option,
                              const QModelIndex &index) const override;

    void setEditorData(QWidget *editor, const QModelIndex &index) const override;
    void setModelData(QWidget *editor, QAbstractItemModel *model,
                          const QModelIndex &index) const override;

     void updateEditorGeometry(QWidget *editor,
            const QStyleOptionViewItem &option, const QModelIndex &index) const override;



};

#endif // MYDELEGATE_H
