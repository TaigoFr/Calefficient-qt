/*
    Elypson/qt-collapsible-section
    (c) 2016 Michael A. Voelkel - michael.alexander.voelkel@gmail.com

    This file is part of Elypson/qt-collapsible section.

    Elypson/qt-collapsible-section is free software: you can redistribute it and/or modify
    it under the terms of the GNU Lesser General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    Elypson/qt-collapsible-section is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU Lesser General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with Elypson/qt-collapsible-section. If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef SECTION_H
#define SECTION_H

#include <QFrame>
#include <QGridLayout>
#include <QParallelAnimationGroup>
#include <QScrollArea>
#include <QToolButton>
#include <QWidget>


class Collapsible : public QWidget {
    Q_OBJECT

signals:
    void toggled();

public slots:
    void toggle(bool collapsed);

public:
    explicit Collapsible(QWidget* parent = 0);

    void setContentLayout(QLayout* contentLayout);
    void setHeader(QWidget* widget);

private:
    QGridLayout* mainLayout;
    QToolButton* toggleButton;
    QParallelAnimationGroup* toggleAnimation;
    QScrollArea* contentArea;
    QWidget* header;

    int animationDuration = 100; // ms
    QIcon right_arrow_icon;
    QIcon down_arrow_icon;

};

#endif // SECTION_H
