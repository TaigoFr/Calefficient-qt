#ifndef SETTINGS_HPP
#define SETTINGS_HPP

#include "scrollabletablewidget.hpp"

#include <QSettings>
#include <QPushButton>


class SettingsPage : public ScrollableTableWidget
{
public:
    SettingsPage(QWidget* parent = nullptr);

    void updateStyle();

public:
    QSettings data;

private:
    QPushButton* reset_app_button;
};

#endif // SETTINGS_HPP
