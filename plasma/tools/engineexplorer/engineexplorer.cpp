/*
 *   Copyright 2007 Aaron Seigo <aseigo@kde.org>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as
 *   published by the Free Software Foundation; either version 2,
 *   or (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details
 *
 *   You should have received a copy of the GNU Library General Public
 *   License along with this program; if not, write to the
 *   Free Software Foundation, Inc.,
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#include "engineexplorer.h"

#include <QApplication>
#include <QStandardItemModel>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QSpinBox>
#include <QBitArray>
#include <QBitmap>

#include <KAction>
#include <KIconLoader>
#include <KIconTheme>
#include <KStandardAction>

#ifdef FOUND_SOPRANO
#include <Soprano/Node>
Q_DECLARE_METATYPE(Soprano::Node)
#endif // FOUND_SOPRANO

#include "plasma/dataenginemanager.h"

EngineExplorer::EngineExplorer(QWidget* parent)
    : KDialog(parent),
      m_engine(0),
      m_sourceCount(0),
      m_requestingSource(false)
{
    setButtons(0);
#ifdef FOUND_SOPRANO
    (void) qRegisterMetaType<Soprano::Node>();
#endif
    setWindowTitle(i18n("Plasma Engine Explorer"));
    QWidget* mainWidget = new QWidget(this);
    setMainWidget(mainWidget);
    setupUi(mainWidget);

    m_engineManager = Plasma::DataEngineManager::self();
    m_dataModel = new QStandardItemModel(this);
    KIcon pix("plasma");
    int size = IconSize(KIconLoader::Dialog);
    m_title->setPixmap(pix.pixmap(size, size));
    connect(m_engines, SIGNAL(activated(QString)), this, SLOT(showEngine(QString)));
    connect(m_sourceRequesterButton, SIGNAL(clicked(bool)), this, SLOT(requestSource()));
    m_data->setModel(m_dataModel);

    m_searchLine->setTreeView(m_data);
    m_searchLine->setClickMessage(i18n("Search"));

    listEngines();
    m_engines->setFocus();

    setButtons(KDialog::Close | KDialog::User1 | KDialog::User2);
    setButtonText(KDialog::User1, i18n("Collapse all"));
    setButtonText(KDialog::User2, i18n("Expand all"));
    connect(this, SIGNAL(user1Clicked()), m_data, SLOT(collapseAll()));
    connect(this, SIGNAL(user2Clicked()), m_data, SLOT(expandAll()));
    enableButton(KDialog::User1, false);
    enableButton(KDialog::User2, false);

    addAction(KStandardAction::quit(qApp, SLOT(quit()), this));
}

EngineExplorer::~EngineExplorer()
{
}

void EngineExplorer::setEngine(const QString &engine)
{
    //find the engine in the combo box
    int index = m_engines->findText(engine);
    if (index != -1) {
        kDebug() << "Engine found!";
        m_engines->setCurrentIndex(index);
        showEngine(engine);
    }
}

void EngineExplorer::setInterval(const int interval)
{
    m_updateInterval->setValue(interval);
}

void EngineExplorer::dataUpdated(const QString& source, const Plasma::DataEngine::Data& data)
{
    QList<QStandardItem*> items = m_dataModel->findItems(source, 0);

    if (items.count() < 1) {
        return;
    }

    QStandardItem* parent = items.first();

    while (parent->hasChildren()) {
        parent->removeRow(0);
    }

    showData(parent, data);
}

void EngineExplorer::listEngines()
{
    m_engines->clear();
    m_engines->addItem("");
    QStringList engines = m_engineManager->listAllEngines();
    qSort(engines);
    m_engines->addItems(engines);
}

void EngineExplorer::showEngine(const QString& name)
{
    m_sourceRequester->setEnabled(false);
    m_sourceRequesterButton->setEnabled(false);
    enableButton(KDialog::User1, false);
    enableButton(KDialog::User2, false);
    m_dataModel->clear();
    m_dataModel->setColumnCount(4);
    QStringList headers;
    headers << i18n("DataSource") << i18n("Key") << i18n("Value") << i18n("Type");
    m_dataModel->setHorizontalHeaderLabels(headers);
    m_engine = 0;
    m_sourceCount = 0;

    if (!m_engineName.isEmpty()) {
        m_engineManager->unloadEngine(m_engineName);
    }

    m_engineName = name;
    if (m_engineName.isEmpty()) {
        updateTitle();
        return;
    }

    m_engine = m_engineManager->loadEngine(m_engineName);
    if (!m_engine) {
        m_engineName.clear();
        updateTitle();
        return;
    }

    QStringList sources = m_engine->sources();

    //kDebug() << "showing engine " << m_engine->objectName();
    //kDebug() << "we have " << sources.count() << " data sources";
    foreach (const QString& source, sources) {
        //kDebug() << "adding " << source;
        addSource(source);
    }

    m_sourceRequesterButton->setEnabled(true);
    m_updateInterval->setEnabled(true);
    m_sourceRequester->setEnabled(true);
    m_sourceRequester->setFocus();
    connect(m_engine, SIGNAL(sourceAdded(QString)), this, SLOT(addSource(QString)));
    connect(m_engine, SIGNAL(sourceRemoved(QString)), this, SLOT(removeSource(QString)));
    updateTitle();
}

void EngineExplorer::addSource(const QString& source)
{
    QStandardItem* parent = new QStandardItem(source);
    m_dataModel->appendRow(parent);

    //kDebug() << "getting data for source " << source;
    Plasma::DataEngine::Data data = m_engine->query(source);
    showData(parent, data);

    if (!m_requestingSource || m_sourceRequester->text() != source) {
        m_engine->connectSource(source, this);
    }

    ++m_sourceCount;
    updateTitle();

    enableButton(KDialog::User1, true);
    enableButton(KDialog::User2, true);
}

void EngineExplorer::removeSource(const QString& source)
{
    QList<QStandardItem*> items = m_dataModel->findItems(source, 0);

    if (items.count() < 1) {
        return;
    }

    foreach (QStandardItem* item, items) {
        m_dataModel->removeRow(item->row());
    }

    --m_sourceCount;
    updateTitle();
}

void EngineExplorer::requestSource()
{
    if (!m_engine) {
        return;
    }

    QString source = m_sourceRequester->text();

    if (source.isEmpty()) {
        return;
    }

    kDebug() << "request source" << source;
    m_requestingSource = true;
    m_engine->connectSource(source, this, (uint)m_updateInterval->value());
    m_requestingSource = false;
}

QString EngineExplorer::convertToString(const QVariant &value) const
{
    switch (value.type())
    {
        case QVariant::BitArray: {
            return i18np("&lt;1 bit&gt;", "&lt;%1 bits&gt;", value.toBitArray().size());
        }
        case QVariant::Bitmap: {
            QBitmap bitmap = value.value<QBitmap>();
            return QString("<%1x%2px - %3bpp>").arg(bitmap.width()).arg(bitmap.height()).arg(bitmap.depth());
        }
        case QVariant::ByteArray: {
            // Return the array size if it is not displayable
            if (value.toString().isEmpty()) {
                return i18np("&lt;1 byte&gt;", "&lt;%1 bytes&gt;", value.toByteArray().size());
            }
            else {
                return value.toString();
            }
        }
        case QVariant::Image: {
            QImage image = value.value<QImage>();
            return QString("<%1x%2px - %3bpp>").arg(image.width()).arg(image.height()).arg(image.depth());
        }
        case QVariant::Line: {
           QLine line = value.toLine();
           return QString("<x1:%1, y1:%2, x2:%3, y2:%4>").arg(line.x1()).arg(line.y1()).arg(line.x2()).arg(line.y2());
        }
        case QVariant::LineF: {
           QLineF lineF = value.toLineF();
           return QString("<x1:%1, y1:%2, x2:%3, y2:%4>").arg(lineF.x1()).arg(lineF.y1()).arg(lineF.x2()).arg(lineF.y2());
        }
        case QVariant::Locale: {
            return QString("%1").arg(value.toLocale().name());
        }
        case QVariant::Map: {
            return i18np("&lt;1 item&gt;", "&lt;%1 items&gt;", value.toMap().size());
        }
        case QVariant::Pixmap: {
            QPixmap pixmap = value.value<QPixmap>();
            return QString("<%1x%2px - %3bpp>").arg(pixmap.width()).arg(pixmap.height()).arg(pixmap.depth());
        }
        case QVariant::Point: {
           QPoint point = value.toPoint();
           return QString("<x:%1, y:%2>").arg(point.x()).arg(point.y());
        }
        case QVariant::PointF: {
           QPointF pointF = value.toPointF();
           return QString("<x:%1, y:%2>").arg(pointF.x()).arg(pointF.y());
        }
        case QVariant::Rect: {
            QRect rect = value.toRect();
            return QString("<x:%1, y:%2, w:%3, h:%4>").arg(rect.x()).arg(rect.y()).arg(rect.width()).arg(rect.height());
        }
        case QVariant::RectF: {
            QRectF rectF = value.toRectF();
            return QString("<x:%1, y:%2, w:%3, h:%4>").arg(rectF.x()).arg(rectF.y()).arg(rectF.width()).arg(rectF.height());
        }
        case QVariant::RegExp: {
            return QString("%1").arg(value.toRegExp().pattern());
        }
        case QVariant::Region: {
            QRect region = value.value<QRegion>().boundingRect();
            return QString("<x:%1, y:%2, w:%3, h:%4>").arg(region.x()).arg(region.y()).arg(region.width()).arg(region.height());
        }
        case QVariant::Size: {
            QSize size = value.toSize();
            return QString("<w:%1, h:%2>").arg(size.width()).arg(size.height());
        }
        case QVariant::SizeF: {
            QSizeF sizeF = value.toSizeF();
            return QString("<w:%1, h:%2>").arg(sizeF.width()).arg(sizeF.height());
        }
        case QVariant::Url: {
            return QString("%1").arg(value.toUrl().toString());
        }
        default: {
#ifdef FOUND_SOPRANO
            if (QLatin1String(value.typeName()) == "Soprano::Node") {
                Soprano::Node node = value.value<Soprano::Node>();
                if (node.isLiteral()) {
                    return convertToString(node.literal().variant());
                } else if (node.isResource()) {
                    return node.uri().toString();
                } else if (node.isBlank()) {
                    return QString("_:%1").arg(node.identifier());
                }
            }
#endif
            if (value.canConvert(QVariant::String)) {
                if (value.toString().isEmpty()) {
                    return i18n("<empty>");
                }
                else {
                    return value.toString();
                }
            }

            return i18n("<not displayable>");
        }
    }
}

void EngineExplorer::showData(QStandardItem* parent, Plasma::DataEngine::Data data)
{
    int rowCount = 0;
    Plasma::DataEngine::DataIterator it(data);
//    parent->insertRows(0, data.count());
//    parent->setColumnCount(3);
    while (it.hasNext()) {
        it.next();
        parent->setChild(rowCount, 1, new QStandardItem(it.key()));
        if (it.value().canConvert(QVariant::List)) {
            foreach(const QVariant &var, it.value().toList()) {
                parent->setChild(rowCount, 2, new QStandardItem(convertToString(var)));
                parent->setChild(rowCount, 3, new QStandardItem(var.typeName()));
                ++rowCount;
            }
        }
        else {
            parent->setChild(rowCount, 2, new QStandardItem(convertToString(it.value())));
            parent->setChild(rowCount, 3, new QStandardItem(it.value().typeName()));
            ++rowCount;
        }
    }
}

void EngineExplorer::updateTitle()
{
    if (!m_engine) {
        m_title->setPixmap(KIcon("plasma").pixmap(IconSize(KIconLoader::Dialog)));
        m_title->setText(i18n("Plasma DataEngine Explorer"));
        return;
    }

    m_title->setText(ki18ncp("The name of the engine followed by the number of data sources",
                             "%1 Engine - 1 data source", "%1 - %2 data sources")
                              .subs(m_engine->objectName().isEmpty() ? i18n("Unnamed")
                                                                     : m_engine->objectName())
                              .subs(m_sourceCount).toString());
    if (m_engine->icon().isEmpty()) {
        m_title->setPixmap(KIcon("plasma").pixmap(IconSize(KIconLoader::Dialog)));
    } else {
        //m_title->setPixmap(KIcon("alarmclock").pixmap(IconSize(KIconLoader::Dialog)));
        m_title->setPixmap(KIcon(m_engine->icon()).pixmap(IconSize(KIconLoader::Dialog)));
    }
}

#include "engineexplorer.moc"

