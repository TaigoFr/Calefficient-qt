#include "settingspage.hpp"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QComboBox>
#include <QLabel>
#include <QCheckBox>

SettingsPage::SettingsPage(QWidget* parent) :
    ScrollableTableWidget(parent)
  , data("Calefficient", "Settings")
{
    /*
    listWidget->setStyleSheet("font: 30px");
    //listWidget->setDragDropMode(QAbstractItemView::InternalMove);
    //listWidget->setAcceptDrops(true);
    //listWidget->setDragEnabled(true);
    */

    QString week_day_setting = data.value("week_day").toString();
    QString auto_update_setting = data.value("auto_update").toString();
    QString calculate_empty_slots_setting = data.value("calculate_empty_slots").toString();
    QString warn_empty_slots_setting = data.value("warn_empty_slots").toString();
    QString ignore_24h_events_setting = data.value("ignore_24h_events").toString();
    QString min_time_setting = data.value("min_time").toString();

    setColumnCount(1);

    /*
    Collapsible* priorities_section = new Collapsible(settings_widget);
    QLabel* priorities_section_header_label = new QLabel("Priorities", priorities_section);
    QVBoxLayout* priorities_section_content_layout = new QVBoxLayout();
    QListWidget *priorities_section_listWidget = new QListWidget(priorities_section);
    priorities_section_listWidget->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    priorities_section_listWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    QVector<GoogleCalendar::Calendar> &calendars = google.getOwnedCalendarList();
    foreach(auto& calendar, calendars){
        QListWidgetItem* item = new QListWidgetItem(calendar.name, priorities_section_listWidget);
        priorities_section_listWidget->addItem(item);
    }
    priorities_section_listWidget->setFrameShape(QFrame::NoFrame);
    priorities_section_content_layout->setContentsMargins(0, 0, 0, 0);
    priorities_section->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    connect(priorities_section, &Collapsible::toggled, [=](){
        settings_widget->resizeRowsToContents();
    });
    priorities_section_content_layout->addWidget(priorities_section_listWidget);
    priorities_section->setHeader(priorities_section_header_label);
    priorities_section->setContentLayout(priorities_section_content_layout);
    priorities_section_listWidget->setSizeAdjustPolicy(QListWidget::AdjustToContents);
    */

    QWidget* week_day_widget = new QWidget(this);
    QHBoxLayout* week_day_layout = new QHBoxLayout();
    week_day_widget->setLayout(week_day_layout);
    week_day_layout->addWidget(new QLabel("Week starting day"));
    QSpacerItem* week_day_spacer = new QSpacerItem(0, 0, QSizePolicy::Expanding, QSizePolicy::Ignored);
    week_day_layout->addSpacerItem(week_day_spacer);
    QComboBox* week_day_combobox = new QComboBox(week_day_widget);
    week_day_combobox->addItems(QStringList() << "Saturday" << "Sunday" << "Monday");
    //week_day_combobox->setStyleSheet("QComboBox { border: 1px solid black; border-radius: 3px;  }"
    //                                 "QComboBox::drop-down { border-top-right-radius: 3px; border-bottom-right-radius: 3px; }"
    //                                 "QComboBox::down-arrow { image: url(:/resources/images/down_arrow.png); width: 12; height: 12; }");
    if(week_day_setting != "")
        week_day_combobox->setCurrentText(week_day_setting);
    else {
        week_day_combobox->setCurrentText("Monday");
        data.setValue("week_day", "Monday");
    }
    connect(week_day_combobox, &QComboBox::currentTextChanged, [this](const QString &text){
        data.setValue("week_day", text);
    });
    week_day_layout->addWidget(week_day_combobox);

    QWidget* auto_update_widget = new QWidget(this);
    QHBoxLayout* auto_update_layout = new QHBoxLayout();
    auto_update_widget->setLayout(auto_update_layout);
    auto_update_layout->addWidget(new QLabel("Auto update"));
    QSpacerItem* auto_update_spacer = new QSpacerItem(0, 0, QSizePolicy::Expanding, QSizePolicy::Ignored);
    auto_update_layout->addSpacerItem(auto_update_spacer);
    QCheckBox* auto_update_checkbox = new QCheckBox(auto_update_widget);
    if(auto_update_setting != "")
        auto_update_checkbox->setCheckState(auto_update_setting == "0" ? Qt::CheckState::Unchecked : Qt::CheckState::Checked);
    else {
        auto_update_checkbox->setCheckState(Qt::CheckState::Unchecked);
        data.setValue("auto_update", "0");
    }
    connect(auto_update_checkbox, &QCheckBox::stateChanged, [this](int state){
        data.setValue("auto_update", QString::number(state));
    });
    auto_update_layout->addWidget(auto_update_checkbox);

    QWidget* calculate_empty_slots_widget = new QWidget(this);
    QHBoxLayout* calculate_empty_slots_layout = new QHBoxLayout();
    calculate_empty_slots_widget->setLayout(calculate_empty_slots_layout);
    calculate_empty_slots_layout->addWidget(new QLabel("Calculate empty slots"));
    QSpacerItem* calculate_empty_slots_spacer = new QSpacerItem(0, 0, QSizePolicy::Expanding, QSizePolicy::Ignored);
    calculate_empty_slots_layout->addSpacerItem(calculate_empty_slots_spacer);
    QCheckBox* calculate_empty_slots_checkbox = new QCheckBox(calculate_empty_slots_widget);
    if(calculate_empty_slots_setting != "")
        calculate_empty_slots_checkbox->setCheckState(calculate_empty_slots_setting == "0" ? Qt::CheckState::Unchecked : Qt::CheckState::Checked);
    else {
        calculate_empty_slots_checkbox->setCheckState(Qt::CheckState::Unchecked);
        data.setValue("calculate_empty_slots", "0");
    }
    connect(calculate_empty_slots_checkbox, &QCheckBox::stateChanged, [this](int state){
        data.setValue("calculate_empty_slots", QString::number(state));
    });
    calculate_empty_slots_layout->addWidget(calculate_empty_slots_checkbox);

    QWidget* warn_empty_slots_widget = new QWidget(this);
    QHBoxLayout* warn_empty_slots_layout = new QHBoxLayout();
    warn_empty_slots_widget->setLayout(warn_empty_slots_layout);
    warn_empty_slots_layout->addWidget(new QLabel("Warn empty slots"));
    QSpacerItem* warn_empty_slots_spacer = new QSpacerItem(0, 0, QSizePolicy::Expanding, QSizePolicy::Ignored);
    warn_empty_slots_layout->addSpacerItem(warn_empty_slots_spacer);
    QCheckBox* warn_empty_slots_checkbox = new QCheckBox(warn_empty_slots_widget);
    if(warn_empty_slots_setting != "")
        warn_empty_slots_checkbox->setCheckState(warn_empty_slots_setting == "0" ? Qt::CheckState::Unchecked : Qt::CheckState::Checked);
    else {
        warn_empty_slots_checkbox->setCheckState(Qt::CheckState::Unchecked);
        data.setValue("warn_empty_slots", "0");
    }
    connect(warn_empty_slots_checkbox, &QCheckBox::stateChanged, [this](int state){
        data.setValue("warn_empty_slots", QString::number(state));
    });
    warn_empty_slots_layout->addWidget(warn_empty_slots_checkbox);

    QWidget* ignore_24h_events_widget = new QWidget(this);
    QHBoxLayout* ignore_24h_events_layout = new QHBoxLayout();
    ignore_24h_events_widget->setLayout(ignore_24h_events_layout);
    ignore_24h_events_layout->addWidget(new QLabel("Ignore 24h+ events"));
    QSpacerItem* ignore_24h_events_spacer = new QSpacerItem(0, 0, QSizePolicy::Expanding, QSizePolicy::Ignored);
    ignore_24h_events_layout->addSpacerItem(ignore_24h_events_spacer);
    QCheckBox* ignore_24h_events_checkbox = new QCheckBox(ignore_24h_events_widget);
    if(ignore_24h_events_setting != "")
        ignore_24h_events_checkbox->setCheckState(ignore_24h_events_setting == "0" ? Qt::CheckState::Unchecked : Qt::CheckState::Checked);
    else {
        ignore_24h_events_checkbox->setCheckState(Qt::CheckState::Unchecked);
        data.setValue("ignore_24h_events", "0");
    }
    connect(ignore_24h_events_checkbox, &QCheckBox::stateChanged, [this](int state){
        data.setValue("ignore_24h_events", QString::number(state));
    });
    ignore_24h_events_layout->addWidget(ignore_24h_events_checkbox);

    QWidget* min_time_widget = new QWidget(this);
    QHBoxLayout* min_time_layout = new QHBoxLayout();
    min_time_widget->setLayout(min_time_layout);
    min_time_layout->addWidget(new QLabel("min_time"));
    QSpacerItem* min_time_spacer = new QSpacerItem(0, 0, QSizePolicy::Expanding, QSizePolicy::Ignored);
    min_time_layout->addSpacerItem(min_time_spacer);
    QComboBox* min_time_combobox = new QComboBox(min_time_widget);
    min_time_combobox->addItems(QStringList() << "0.1" << "1" << "5" << "15");
    if(min_time_setting != "")
        min_time_combobox->setCurrentText(min_time_setting);
    else {
        min_time_combobox->setCurrentText("0.1");
        data.setValue("min_time", "0.1");
    }
    connect(min_time_combobox, &QComboBox::currentTextChanged, [this](const QString &text){
        data.setValue("min_time", text);
    });
    min_time_layout->addWidget(min_time_combobox);


    QWidget* reset_app_widget = new QWidget(this);
    reset_app_button = new QPushButton("Delete all data", reset_app_widget);
    QHBoxLayout* reset_app_layout = new QHBoxLayout();
    reset_app_widget->setLayout(reset_app_layout);
    reset_app_layout->addWidget(reset_app_button);
    reset_app_button->setStyleSheet("QPushButton { background-color:red; color: white; }");

    //settings_widget->ddWidget(priorities_section);
    addWidget(week_day_widget);
    addWidget(auto_update_widget);
    addWidget(calculate_empty_slots_widget);
    addWidget(warn_empty_slots_widget);
    addWidget(ignore_24h_events_widget);
    addWidget(min_time_widget);
    addWidget(reset_app_widget);
}

void SettingsPage::updateStyle()
{
    QSize window_size = window()->size();
    setStyleSheet("font: " + QString::number((int)(window_size.height() * 0.025)) + "px;");
    resizeRowsToContents();
    float width = window_size.width() * 0.97;
    reset_app_button->setFixedWidth(width);
}
