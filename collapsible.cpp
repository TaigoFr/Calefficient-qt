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

#include <QPropertyAnimation>
#include <QScrollBar>

#include "collapsible.h"

Collapsible::Collapsible(QWidget *parent)
        : QWidget(parent) {
    toggleButton = new QToolButton(this);
    toggleAnimation = new QParallelAnimationGroup(this);
    contentArea = new QScrollArea(this);
    mainLayout = new QGridLayout(this);

    down_arrow_icon = QIcon(":/resources/images/down_arrow.png");
    right_arrow_icon = QIcon(":/resources/images/right_arrow.png");

    toggleButton->setStyleSheet("QToolButton "
                                "{"
                                "border: none; "
                                "}");
    toggleButton->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    toggleButton->setIcon(right_arrow_icon);
    toggleButton->setCheckable(true);
    toggleButton->setChecked(false);

    contentArea->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    contentArea->setFrameShape(QFrame::NoFrame);
    contentArea->setContentsMargins(0, 0, 0, 0);

    // start out collapsed
    contentArea->setMaximumHeight(0);
    contentArea->setMinimumHeight(0);

    // let the entire widget grow and shrink with its content
    toggleAnimation->addAnimation(new QPropertyAnimation(this, "minimumHeight"));
    toggleAnimation->addAnimation(new QPropertyAnimation(this, "maximumHeight"));
    toggleAnimation->addAnimation(new QPropertyAnimation(contentArea, "maximumHeight"));

    mainLayout->setVerticalSpacing(0);
    mainLayout->setContentsMargins(0, 0, 0, 0);

    mainLayout->addWidget(toggleButton, 0, 0, 1, 1, Qt::AlignLeft);
    mainLayout->addWidget(contentArea, 1, 0, 1, 2);
    setLayout(mainLayout);

    connect(toggleButton, &QToolButton::toggled, this, &Collapsible::toggle);
}


void Collapsible::toggle(bool collapsed) {
    toggleButton->setIcon(collapsed ? down_arrow_icon : right_arrow_icon);
    toggleAnimation->setDirection(collapsed ? QAbstractAnimation::Forward : QAbstractAnimation::Backward);
    toggleAnimation->start();

    connect(toggleAnimation, &QParallelAnimationGroup::finished, [=](){
        emit toggled();
    });
}


void Collapsible::setContentLayout(QLayout* contentLayout) {

    contentArea->setLayout(contentLayout);
    const int collapsedHeight = sizeHint().height() - contentArea->maximumHeight();
    int contentHeight = contentLayout->sizeHint().height();

    for (int i = 0; i < toggleAnimation->animationCount() - 1; ++i) {
        QPropertyAnimation *SectionAnimation = static_cast<QPropertyAnimation *>(toggleAnimation->animationAt(i));
        SectionAnimation->setDuration(animationDuration);
        SectionAnimation->setStartValue(collapsedHeight);
        SectionAnimation->setEndValue(collapsedHeight + contentHeight);
    }

    QPropertyAnimation *contentAnimation = static_cast<QPropertyAnimation *>(toggleAnimation->animationAt(
            toggleAnimation->animationCount() - 1));
    contentAnimation->setDuration(animationDuration);
    contentAnimation->setStartValue(0);
    contentAnimation->setEndValue(contentHeight);
}

void Collapsible::setHeader(QWidget* widget)
{
    header = widget;
    widget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Maximum);
    mainLayout->addWidget(widget, 0, 1, 1, 1);

    toggleButton->setIconSize(QSize(widget->sizeHint().height() / 2, widget->sizeHint().height() / 3));
    toggleButton->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
}
