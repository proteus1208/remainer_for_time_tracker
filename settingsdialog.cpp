#include "settingsdialog.h"

#include <QCheckBox>
#include <QDialogButtonBox>
#include <QFormLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QMessageBox>
#include <QSpinBox>
#include <QVBoxLayout>

SettingsDialog::SettingsDialog(const Values &values, QWidget *parent)
    : QDialog(parent)
    , m_shapesEnabled(new QCheckBox(tr("Spinning shapes"), this))
    , m_shapesInterval(new QSpinBox(this))
    , m_fullscreenEnabled(new QCheckBox(tr("Full-screen color"), this))
    , m_fullscreenInterval(new QSpinBox(this))
{
    setWindowTitle(tr("Remainer Settings"));
    setWindowFlag(Qt::WindowStaysOnTopHint);
    setModal(true);

    auto *intro = new QLabel(
        tr("Choose when each reminder appears after the mouse and keyboard stay idle. "
           "Uncheck a stage to turn it off."),
        this);
    intro->setWordWrap(true);

    m_shapesEnabled->setChecked(values.shapesEnabled);
    m_fullscreenEnabled->setChecked(values.fullscreenEnabled);

    const auto setupInterval = [](QSpinBox *spin, int seconds) {
        spin->setRange(1, 86400);
        spin->setSuffix(tr(" seconds"));
        spin->setValue(seconds);
        spin->setAccelerated(true);
    };
    setupInterval(m_shapesInterval, values.shapesIntervalSec);
    setupInterval(m_fullscreenInterval, values.fullscreenIntervalSec);

    auto *shapesAfter = new QWidget(this);
    auto *shapesRow = new QHBoxLayout(shapesAfter);
    shapesRow->setContentsMargins(0, 0, 0, 0);
    shapesRow->addWidget(new QLabel(tr("After"), shapesAfter));
    shapesRow->addWidget(m_shapesInterval);
    shapesRow->addStretch();

    auto *fullscreenAfter = new QWidget(this);
    auto *fullscreenRow = new QHBoxLayout(fullscreenAfter);
    fullscreenRow->setContentsMargins(0, 0, 0, 0);
    fullscreenRow->addWidget(new QLabel(tr("After"), fullscreenAfter));
    fullscreenRow->addWidget(m_fullscreenInterval);
    fullscreenRow->addStretch();

    shapesAfter->setEnabled(m_shapesEnabled->isChecked());
    fullscreenAfter->setEnabled(m_fullscreenEnabled->isChecked());
    connect(m_shapesEnabled, &QCheckBox::toggled, shapesAfter, &QWidget::setEnabled);
    connect(m_fullscreenEnabled, &QCheckBox::toggled, fullscreenAfter, &QWidget::setEnabled);

    auto *form = new QFormLayout;
    form->addRow(m_shapesEnabled);
    form->addRow(shapesAfter);
    form->addRow(m_fullscreenEnabled);
    form->addRow(fullscreenAfter);

    auto *buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
    connect(buttons, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);

    auto *layout = new QVBoxLayout(this);
    layout->addWidget(intro);
    layout->addLayout(form);
    layout->addWidget(buttons);
}

SettingsDialog::Values SettingsDialog::values() const
{
    Values result;
    result.shapesEnabled = m_shapesEnabled->isChecked();
    result.shapesIntervalSec = m_shapesInterval->value();
    result.fullscreenEnabled = m_fullscreenEnabled->isChecked();
    result.fullscreenIntervalSec = m_fullscreenInterval->value();
    return result;
}

void SettingsDialog::accept()
{
    if (m_shapesEnabled->isChecked() && m_fullscreenEnabled->isChecked()
        && m_fullscreenInterval->value() <= m_shapesInterval->value()) {
        QMessageBox::warning(
            this,
            tr("Settings"),
            tr("Full-screen color must start later than the spinning shapes. "
               "Increase the full-screen time."));
        return;
    }

    QDialog::accept();
}
