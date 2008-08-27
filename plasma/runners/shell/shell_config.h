
#ifndef SHELLCONFIG_H
#define SHELLCONFIG_H

#include <KCModule>
#include "ui_shellOptions.h"


class ShellConfigForm : public QWidget, public Ui::shellOptions
{
    Q_OBJECT
    public:
        explicit ShellConfigForm(QWidget* parent);
};

class ShellConfig : public KCModule
{
    Q_OBJECT
    public:
        explicit ShellConfig(QWidget* parent = 0, const QVariantList& args = QVariantList());
        ~ShellConfig();

    public slots:
        void save();
        void load();
        void defaults();
        void setRunInTerminal(bool);

    private:
        ShellConfigForm* m_ui;
        bool m_inTerminal;
};

#endif
