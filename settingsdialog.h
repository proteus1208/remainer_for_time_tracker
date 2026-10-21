#ifndef SETTINGSDIALOG_H
#define SETTINGSDIALOG_H

#include <QDialog>

class QCheckBox;
class QSpinBox;

class SettingsDialog : public QDialog
{
    Q_OBJECT

public:
    struct Values {
        bool shapesEnabled = true;
        int shapesIntervalSec = 7;
        bool fullscreenEnabled = true;
        int fullscreenIntervalSec = 37;
    };

    explicit SettingsDialog(const Values &values, QWidget *parent = nullptr);

    Values values() const;

protected:
    void accept() override;

private:
    QCheckBox *m_shapesEnabled;
    QSpinBox *m_shapesInterval;
    QCheckBox *m_fullscreenEnabled;
    QSpinBox *m_fullscreenInterval;
};

#endif // SETTINGSDIALOG_H
