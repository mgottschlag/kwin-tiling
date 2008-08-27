

#include "shell_config.h"

#include <QGridLayout>

#include <KConfigGroup>
#include <KDebug>
#include <KPluginFactory>
#include <KPluginLoader>

#include <plasma/abstractrunner.h>

K_EXPORT_RUNNER_CONFIG(shell, ShellConfig)

ShellConfigForm::ShellConfigForm(QWidget* parent) : QWidget(parent)
{
  setupUi(this);
}

ShellConfig::ShellConfig(QWidget* parent, const QVariantList& args) :
    KCModule(ConfigFactory::componentData(), parent, args)
{
    m_ui = new ShellConfigForm(this);

    QGridLayout* layout = new QGridLayout(this);

    layout->addWidget(m_ui, 0, 0);

    connect(m_ui->cbRunInTerminal, SIGNAL(toggled(bool)), this, SLOT(setRunInTerminal(bool)));

    load();
}

ShellConfig::~ShellConfig()
{
}


void ShellConfig::setRunInTerminal(bool inTerminal)
{
    m_inTerminal = inTerminal;
}

void ShellConfig::load()
{
    KCModule::load();

    //TODO load
    emit changed(false);
}

void ShellConfig::save()
{
    //TODO save
    emit changed(false);
}

void ShellConfig::defaults()
{
    //TODO default
    emit changed(true);
}


#include "shell_config.moc"
