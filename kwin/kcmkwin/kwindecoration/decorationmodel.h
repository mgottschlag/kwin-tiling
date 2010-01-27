/********************************************************************
 KWin - the KDE window manager
 This file is part of the KDE project.

Copyright (C) 2009 Martin Gräßlin <kde@martin-graesslin.com>

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*********************************************************************/

#ifndef PREVIEWMODEL_H
#define PREVIEWMODEL_H
#include <QAbstractListModel>
#include <QPixmap>
#include <KConfig>
#include <kdecoration.h>

class KDesktopFile;
class KDecorationPlugins;
class KDecorationPreview;

namespace KWin
{

class AuroraePreview;

class DecorationModelData
{
    public:
        enum DecorationType
            {
            NativeDecoration = 0,
            AuroraeDecoration = 1
            };
        QString name;
        QString libraryName;
        QPixmap preview;
        DecorationType type;
        QString comment;
        QString author;
        QString email;
        QString website;
        QString version;
        QString license;
        QString auroraeName;
        AuroraePreview *aurorae;
        KDecorationDefines::BorderSize borderSize;

        static bool less( const DecorationModelData& a, const DecorationModelData& b )
            { return a.name < b.name; }
};

class DecorationModel : public QAbstractListModel
{
    Q_OBJECT
    public:
        enum
            {
            NameRole = Qt::UserRole,
            LibraryNameRole = Qt::UserRole + 1,
            PixmapRole = Qt::UserRole + 2,
            TypeRole = Qt::UserRole + 3,
            AuroraeNameRole = Qt::UserRole + 4,
            PackageDescriptionRole = Qt::UserRole + 5,
            PackageAuthorRole = Qt::UserRole + 6,
            PackageEmailRole = Qt::UserRole + 7,
            PackageWebsiteRole = Qt::UserRole + 8,
            PackageVersionRole = Qt::UserRole + 9,
            PackageLicenseRole = Qt::UserRole + 10,
            BorderSizeRole = Qt::UserRole + 11,
            BorderSizesRole = Qt::UserRole + 12
            };
        DecorationModel( KSharedConfigPtr config, QObject* parent = 0 );
        ~DecorationModel();

        virtual QVariant data( const QModelIndex& index, int role = Qt::DisplayRole ) const;
        virtual int rowCount( const QModelIndex& parent = QModelIndex() ) const;

        void reload();

        void regeneratePreview( const QModelIndex& index );

        /**
        * Changes the button state and regenerates the preview.
        */
        void changeButtons( bool custom, const QString& left, const QString& right );
        /**
        * Changes the button state without regenerating the preview.
        */
        void setButtons( bool custom, const QString& left, const QString& right );

        void setBorderSize( const QModelIndex& index, KDecorationDefines::BorderSize size );
        void resetAuroraeBorderSizes( KDecorationDefines::BorderSize size );

        QModelIndex indexOfLibrary( const QString& libraryName ) const;
        QModelIndex indexOfName( const QString& decoName ) const;
        QModelIndex indexOfAuroraeName( const QString& auroraeName ) const;

    public slots:
        void regeneratePreview( const QModelIndex& index, const QSize& size );

    private:
        void findDecorations();
        void findAuroraeThemes();
        void regeneratePreviews();
        void metaData( DecorationModelData& data, const KDesktopFile& df );
        QList<DecorationModelData> m_decorations;
        KDecorationPlugins* m_plugins;
        KDecorationPreview* m_preview;
        bool m_customButtons;
        QString m_leftButtons;
        QString m_rightButtons;
};

} // namespace KWin

#endif // PREVIEWMODEL_H