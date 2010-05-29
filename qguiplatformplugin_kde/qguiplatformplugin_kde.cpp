/*  This file is part of the KDE project

    Copyright (C) 2009 Nokia Corporation and/or its subsidiary(-ies). <qt-info@nokia.com>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#include <KDE/KStandardDirs>
#include <KDE/KGlobal>
#include <KDE/KComponentData>
#include <KDE/KGlobalSettings>
#include <KDE/KStyle>
#include <KDE/KConfigGroup>
#include <KDE/KIcon>
#include <KDE/KFileDialog>
#include <KDE/KColorDialog>
#include <QtCore/QHash>
#include <QtGui/QFileDialog>
#include <QtGui/QColorDialog>
#include <QtGui/QApplication>
#include <QtGui/QToolButton>
#include "qguiplatformplugin_p.h"

#include <kdebug.h>

/*
 * Map a Qt filter string into a KDE one.
 * (from kfiledialog.cpp)
*/
static QString qt2KdeFilter(const QString &f)
{
    QString filter;
    QTextStream str(&filter, QIODevice::WriteOnly);
    const QStringList list(f.split(";;").replaceInStrings("/", "\\/"));
    QStringList::const_iterator it(list.begin()),
                                end(list.end());
    bool first=true;

    for(; it!=end; ++it)
    {
        int ob=(*it).lastIndexOf('('),
            cb=(*it).lastIndexOf(')');

        if(-1!=cb && ob<cb)
        {
            if(first)
                first=false;
            else
                str << '\n';
            str << (*it).mid(ob+1, (cb-ob)-1) << '|' << (*it).mid(0, ob);
        }
    }
    return filter;
}

/*
 * Map a KDE filter string into a Qt one.
 * (from kfiledialog.cpp)
 */
static void kde2QtFilter(const QString &orig, const QString &kde, QString *sel)
{
    if(sel)
    {
        const QStringList list(orig.split(";;"));
        QStringList::const_iterator it(list.begin()),
                                    end(list.end());
        int pos;

        for(; it!=end; ++it)
            if(-1!=(pos=(*it).indexOf(kde)) && pos>0 &&
                ('('==(*it)[pos-1] || ' '==(*it)[pos-1]) &&
                (*it).length()>=kde.length()+pos &&
                (')'==(*it)[pos+kde.length()] || ' '==(*it)[pos+kde.length()]))
            {
                *sel=*it;
                return;
            }
    }
}


class KFileDialogBridge : public KFileDialog
{
public:
    KFileDialogBridge (const KUrl &startDir, const QString &filter, QFileDialog *original_)
     :  KFileDialog (startDir, filter, original_), original(original_)
     {
         connect(this, SIGNAL(fileSelected(QString)), original, SIGNAL(currentChanged(QString)));
     }

    virtual void accept()
    {
        kDebug();
        KFileDialog::accept();
        QMetaObject::invokeMethod(original, "accept"); //workaround protected
    }

    virtual void reject()
    {
        kDebug();
        KFileDialog::reject();
        QMetaObject::invokeMethod(original, "reject"); //workaround protected
    }

    QFileDialog *original;
};

class KColorDialogBridge : public KColorDialog
{
public:
    KColorDialogBridge(QColorDialog* original_ = 0L) : KColorDialog(original_, true) , original(original_)
    {
        connect(this, SIGNAL(colorSelected(QColor)), original, SIGNAL(currentColorChanged(QColor)));
    }

    QColorDialog *original;

    virtual void accept()
    {
        KColorDialog::accept();
        original->setCurrentColor(color());
        QMetaObject::invokeMethod(original, "accept"); //workaround protected
    }

    virtual void reject()
    {
        KColorDialog::reject();
        QMetaObject::invokeMethod(original, "reject"); //workaround protected
    }
};

Q_DECLARE_METATYPE(KFileDialogBridge *)
Q_DECLARE_METATYPE(KColorDialogBridge *)

class KQGuiPlatformPlugin : public QGuiPlatformPlugin
{
    Q_OBJECT
public:
    KQGuiPlatformPlugin()
    {
        connect(KGlobalSettings::self(), SIGNAL(toolbarAppearanceChanged(int)), this, SLOT(updateToolbarStyle()));
        connect(KGlobalSettings::self(), SIGNAL(kdisplayStyleChanged()), this, SLOT(updateWidgetStyle()));
    }

    virtual QStringList keys() const { return QStringList() << QLatin1String("kde"); }
    virtual QString styleName()
    {
        const QString defaultStyle = KStyle::defaultStyle();
        const KConfigGroup pConfig(KGlobal::config(), "General");
        return pConfig.readEntry("widgetStyle", defaultStyle);
    }
    virtual QPalette palette()
    {
        return KGlobalSettings::createApplicationPalette();
    }
    virtual QString systemIconThemeName()
    {
        return KIconTheme::current();
    }
    virtual QStringList iconThemeSearchPaths()
    {
        return KGlobal::dirs()->resourceDirs("icon");
    }
    virtual QIcon fileSystemIcon(const QFileInfo &file)
    {
        return KIcon(KMimeType::findByPath(file.filePath(), 0, true)->iconName());
    }
    virtual int platformHint(PlatformHint hint)
    {
        switch(hint)
        {
        case PH_ToolButtonStyle: {
            KConfigGroup group(KGlobal::config(), "Toolbar style");
            QString style = group.readEntry("ToolButtonStyle", "TextUnderIcon").toLower();
            if (style == "textbesideicon" || style == "icontextright")
                return Qt::ToolButtonTextBesideIcon;
            else if (style == "textundericon" || style == "icontextbottom")
                return Qt::ToolButtonTextUnderIcon;
            else if (style == "textonly")
                return Qt::ToolButtonTextOnly;
            else
                return Qt::ToolButtonIconOnly;
        }
        case PH_ToolBarIconSize:
            return KIconLoader::global()->currentSize(KIconLoader::MainToolbar);
        case PH_ItemView_ActivateItemOnSingleClick:
            return KGlobalSettings::singleClick();
        default:
            break;
        }
        return QGuiPlatformPlugin::platformHint(hint);
    }

public: // File Dialog integration
#define K_FD(QFD) KFileDialogBridge *kdefd = qvariant_cast<KFileDialogBridge *>(QFD->property("_k_bridge"))
    virtual void fileDialogDelete(QFileDialog *qfd)
    {
        K_FD(qfd);
        delete kdefd;
    }
    virtual bool fileDialogSetVisible(QFileDialog *qfd, bool visible)
    {
        K_FD(qfd);
        if (!kdefd && visible) {
            if(qfd->options() & QFileDialog::DontUseNativeDialog)
                return false;

            kdefd = new KFileDialogBridge(KUrl::fromPath(qfd->directory().canonicalPath()),
                                          qt2KdeFilter(qfd->nameFilters().join(";;")), qfd);

            qfd->setProperty("_k_bridge", QVariant::fromValue(kdefd));
        }

        if (visible) {
            switch (qfd->fileMode()) {
                case QFileDialog::AnyFile:
                    kdefd->setMode(KFile::LocalOnly | KFile::File);
                    break;
                case QFileDialog::ExistingFile:
                    kdefd->setMode(KFile::LocalOnly | KFile::File | KFile::ExistingOnly);
                    break;
                case QFileDialog::ExistingFiles:
                    kdefd->setMode(KFile::LocalOnly | KFile::Files | KFile::ExistingOnly);
                    break;
                case QFileDialog::Directory:
                case QFileDialog::DirectoryOnly:
                    kdefd->setMode(KFile::LocalOnly | KFile::Directory);
                    break;
            }


            kdefd->setOperationMode((qfd->acceptMode() == QFileDialog::AcceptSave) ? KFileDialog::Saving : KFileDialog::Opening);
            kdefd->setCaption(qfd->windowTitle());
            kdefd->setConfirmOverwrite(qfd->confirmOverwrite());
            kdefd->setSelection(qfd->selectedFiles().value(0));
        }
        kdefd->setVisible(visible);
        return true;
    }
    virtual QDialog::DialogCode fileDialogResultCode(QFileDialog *qfd)
    {
        K_FD(qfd);
        Q_ASSERT(kdefd);
        return QDialog::DialogCode(kdefd->result());
    }
    virtual void fileDialogSetDirectory(QFileDialog *qfd, const QString &directory)
    {
        K_FD(qfd);
        kdefd->setUrl(KUrl::fromPath(directory));
    }
    virtual QString fileDialogDirectory(const QFileDialog *qfd) const
    {
        K_FD(qfd);
        Q_ASSERT(kdefd);
        return kdefd->baseUrl().pathOrUrl();
    }
    virtual void fileDialogSelectFile(QFileDialog *qfd, const QString &filename)
    {
        K_FD(qfd);
        Q_ASSERT(kdefd);
        kdefd->setSelection(filename);
    }
    virtual QStringList fileDialogSelectedFiles(const QFileDialog *qfd) const
    {
        K_FD(qfd);
        Q_ASSERT(kdefd);
        return kdefd->selectedFiles();
    }
    /*virtual void fileDialogSetFilter(QFileDialog *qfd)
    {
        K_FD(qfd);
    }*/
    virtual void fileDialogSetNameFilters(QFileDialog *qfd, const QStringList &filters)
    {
        K_FD(qfd);
        Q_ASSERT(kdefd);
        kdefd->setFilter(qt2KdeFilter(filters.join(";;")));
    }
    /*virtual void fileDialogSelectNameFilter(QFileDialog *qfd, const QString &filter)
    {
        K_FD(qfd);
    }*/
    virtual QString fileDialogSelectedNameFilter(const QFileDialog *qfd) const
    {
        K_FD(qfd);
        Q_ASSERT(kdefd);
        QString ret;
        kde2QtFilter(qfd->nameFilters().join(";;"), kdefd->currentFilter(), &ret);
        return ret;
    }
public: // ColorDialog
#define K_CD(QCD) KColorDialogBridge *kdecd = qvariant_cast<KColorDialogBridge *>(QCD->property("_k_bridge"))
    virtual void colorDialogDelete(QColorDialog *qcd)
    {
        K_CD(qcd);
        delete kdecd;

    }
    virtual bool colorDialogSetVisible(QColorDialog *qcd, bool visible)
    {
        K_CD(qcd);
        if (!kdecd) {
            kdecd = new KColorDialogBridge(qcd);
            kdecd->setColor(qcd->currentColor());
            if (qcd->options() & QColorDialog::NoButtons) {
                kdecd->setButtons(KDialog::None);
            }
            kdecd->setModal(qcd->isModal());
            qcd->setProperty("_k_bridge", QVariant::fromValue(kdecd));
        }
        if (visible) {
            kdecd->setCaption(qcd->windowTitle());
            kdecd->setAlphaChannelEnabled(qcd->options() & QColorDialog::ShowAlphaChannel);
        }
        kdecd->setVisible(visible);
        return true;
    }
    virtual void colorDialogSetCurrentColor(QColorDialog *qcd, const QColor &color)
    {
        K_CD(qcd);
        if (kdecd) {
            kdecd->setColor(color);
        }
    }

private slots:
    void updateToolbarStyle()
    {
        //from gtksymbol.cpp
        QWidgetList widgets = QApplication::allWidgets();
        for (int i = 0; i < widgets.size(); ++i) {
            QWidget *widget = widgets.at(i);
            if (qobject_cast<QToolButton*>(widget)) {
                QEvent event(QEvent::StyleChange);
                QApplication::sendEvent(widget, &event);
            }
        }
    }

    void updateWidgetStyle()
    {
        if (qApp) {
            if (qApp->style()->objectName() != styleName()) {
                qApp->setStyle(styleName());
            }
        }
    }
};

Q_EXPORT_PLUGIN2(KQGuiPlatformPlugin, KQGuiPlatformPlugin)

#include "qguiplatformplugin_kde.moc"

