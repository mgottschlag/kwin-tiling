

#ifndef ENGINEEXPLORER_H
#define ENGINEEXPLORER_H

class QStandardItemModel;
class QStandardItem;

#include "dataengine.h"
#include "datavisualization.h"

#include "ui_engineexplorer.h"

class DataEngineManager;
namespace Plasma
{
    class DataEngine;
} // namespace Plasma

class EngineExplorer : public KDialog, public Ui::EngineExplorer
{
    Q_OBJECT

    public:
        explicit EngineExplorer(QWidget *parent = 0);
        ~EngineExplorer();

    public slots:
        void updated(const QString& source, const Plasma::DataEngine::Data& data);

    private slots:
        void showEngine(const QString& engine);
        void addSource(const QString& source);
        void removeSource(const QString& source);

    private:
        void listEngines();
        void showData(QStandardItem* parent, Plasma::DataEngine::Data data);

        DataEngineManager* m_engineManager;
        QStandardItemModel* m_dataModel;
        QString m_engineName;
        Plasma::DataEngine* m_engine;
};

#endif // multiple inclusion guard

