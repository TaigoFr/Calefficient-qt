#ifndef AUTOARRANGEDGRIDLAYOUT_HPP
#define AUTOARRANGEDGRIDLAYOUT_HPP

#include <QGridLayout>
#include <QWidget>
#include <QVector>

#include "qgridlayoututil.hpp"

class AutoArrangedGridLayout : public QGridLayout
{
public:
    AutoArrangedGridLayout(QWidget *parent = nullptr);
    ~AutoArrangedGridLayout();

    void addWidget(QWidget *widget, Qt::Alignment alignment = Qt::Alignment());

    void removeWidget(QWidget *);
    void removeItem(QLayoutItem *, bool a_reorder = true, bool a_delete = false);
    void removeAll(bool a_delete = false);

    void switchWidgets(QLayoutItem* t1, QLayoutItem *t2);

    inline void setColumns(int c) { cols = c; }
    inline int getColumns() const  { return cols; }

    inline int getRows() const  { return (count - 1) / cols + 1; }
    inline int getCount() const { return count; }

private:
    int cols;
    int count;
};

#endif // AUTOARRANGEDGRIDLAYOUT_HPP
