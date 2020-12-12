#include "autoarrangedgridlayout.hpp"
#include <QDebug>
#include <QVBoxLayout>
#include <QSpacerItem>

AutoArrangedGridLayout::AutoArrangedGridLayout(QWidget *parent) : QGridLayout(parent), cols(0), count(0)
{

}

AutoArrangedGridLayout::~AutoArrangedGridLayout()
{
}

void AutoArrangedGridLayout::addWidget(QWidget *widget, Qt::Alignment alignment)
{
    this->QGridLayout::addWidget(widget, (int)(count / this->cols), count % this->cols, alignment);
    count++;
}

void AutoArrangedGridLayout::removeWidget(QWidget *widget)
{
    this->removeItem(this->itemAt(this->indexOf(widget)));
}

void AutoArrangedGridLayout::removeItem(QLayoutItem *item, bool a_reorder, bool a_delete)
{

    int index = this->indexOf(item);
    if(a_delete){
        item->widget()->hide();
        QGridLayoutUtil::removeCell(this, index / this->cols, index  % this->cols, true);
    } else {
        item->widget()->setEnabled(false);
        this->QGridLayout::removeItem(item);
    }
    count--;

    if(a_reorder){
        for (int i = index; i < count; ++i){
            QLayoutItem *t2 = this->itemAt(index);
            this->QGridLayout::removeItem(t2);
            this->QGridLayout::addWidget(t2->widget(), i / this->cols, i % this->cols);
        }
    }
}

void AutoArrangedGridLayout::removeAll(bool a_delete)
{
    int c = count;
    for (int i = 0; i < c; ++i)
        this->removeItem(this->itemAt(0), false, a_delete);
}

void AutoArrangedGridLayout::switchWidgets(QLayoutItem *t1, QLayoutItem *t2)
{
    int index1 = this->indexOf(t1);
    int index2 = this->indexOf(t2);

    this->QGridLayout::removeItem(t1);
    this->QGridLayout::removeItem(t2);

    this->QGridLayout::addWidget(t1->widget(), index2 / this->cols, index2 % this->cols);
    this->QGridLayout::addWidget(t2->widget(), index1 / this->cols, index1  % this->cols);
}

