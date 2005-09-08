/***************************************************************************
 *   Copyright (C) 2001 by Matthias Hoelzer-Kluepfel <mhk@caldera.de>      *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <q3groupbox.h>
#include <q3header.h>
#include <qlayout.h>
#include <q3listview.h>
#include <qsplitter.h>
#include <q3textview.h>
#include <qtimer.h>
//Added by qt3to4:
#include <QVBoxLayout>
#include <Q3ValueList>

#include <kaboutdata.h>
#include <kdialog.h>
#include <kgenericfactory.h>

#include "usbdevices.h"
#include "kcmusb.moc"

typedef KGenericFactory<USBViewer, QWidget > USBFactory;
K_EXPORT_COMPONENT_FACTORY (kcm_usb, USBFactory("kcmusb") )

USBViewer::USBViewer(QWidget *parent, const char *, const QStringList &)
  : KCModule(USBFactory::instance(), parent)
{
  setButtons(Help);

  setQuickHelp( i18n("<h1>USB Devices</h1> This module allows you to see"
     " the devices attached to your USB bus(es)."));

  QVBoxLayout *vbox = new QVBoxLayout(this, 0, KDialog::spacingHint());
  Q3GroupBox *gbox = new Q3GroupBox(i18n("USB Devices"), this);
  gbox->setColumnLayout( 0, Qt::Horizontal );
  vbox->addWidget(gbox);

  QVBoxLayout *vvbox = new QVBoxLayout(gbox->layout(), KDialog::spacingHint());

  QSplitter *splitter = new QSplitter(gbox);
  vvbox->addWidget(splitter);

  _devices = new Q3ListView(splitter);
  _devices->addColumn(i18n("Device"));
  _devices->setRootIsDecorated(true);
  _devices->header()->hide();
  _devices->setMinimumWidth(200);
  _devices->setColumnWidthMode(0, Q3ListView::Maximum);

  Q3ValueList<int> sizes;
  sizes.append(200);
  splitter->setSizes(sizes);

  _details = new Q3TextView(splitter);

  splitter->setResizeMode(_devices, QSplitter::KeepSize);

  QTimer *refreshTimer = new QTimer(this);
  // 1 sec seems to be a good compromise between latency and polling load.
  refreshTimer->start(1000);

  connect(refreshTimer, SIGNAL(timeout()), SLOT(refresh()));
  connect(_devices, SIGNAL(selectionChanged(Q3ListViewItem*)),
	  this, SLOT(selectionChanged(Q3ListViewItem*)));

  KAboutData *about =
  new KAboutData(I18N_NOOP("kcmusb"), I18N_NOOP("KDE USB Viewer"),
                0, 0, KAboutData::License_GPL,
                I18N_NOOP("(c) 2001 Matthias Hoelzer-Kluepfel"));

  about->addAuthor("Matthias Hoelzer-Kluepfel", 0, "mhk@kde.org");
  about->addCredit("Leo Savernik", "Live Monitoring of USB Bus", "l.savernik@aon.at");
  setAboutData( about );

  load();
}

void USBViewer::load()
{
  _items.clear();
  _devices->clear();

  refresh();
}

static Q_UINT32 key( USBDevice &dev )
{
  return dev.bus()*256 + dev.device();
}

static Q_UINT32 key_parent( USBDevice &dev )
{
  return dev.bus()*256 + dev.parent();
}

static void delete_recursive( Q3ListViewItem *item, const Q3IntDict<Q3ListViewItem> &new_items )
{
  if (!item)
	return;

  Q3ListViewItemIterator it( item );
  while ( it.current() ) {
        if (!new_items.find(it.current()->text(1).toUInt())) {
		delete_recursive( it.current()->firstChild(), new_items);
		delete it.current();
	}
	++it;
  }
}

void USBViewer::refresh()
{
  Q3IntDict<Q3ListViewItem> new_items;

  if (!USBDevice::parse("/proc/bus/usb/devices"))
    USBDevice::parse("/proc/bus/usb/devices_please-use-sysfs-instead");

  int level = 0;
  bool found = true;

  while (found)
    {
      found = false;

      Q3PtrListIterator<USBDevice> it(USBDevice::devices());
      for ( ; it.current(); ++it)
	if (it.current()->level() == level)
	  {
	    Q_UINT32 k = key(*it.current());
	    if (level == 0)
	      {
		Q3ListViewItem *item = _items.find(k);
		if (!item) {
		    item = new Q3ListViewItem(_devices,
				it.current()->product(),
				QString::number(k));
		}
		new_items.insert(k, item);
		found = true;
	      }
	    else
	      {
		Q3ListViewItem *parent = new_items.find(key_parent(*it.current()));
		if (parent)
		  {
		    Q3ListViewItem *item = _items.find(k);

		    if (!item) {
		        item = new Q3ListViewItem(parent,
				    it.current()->product(),
				    QString::number(k) );
		    }
		    new_items.insert(k, item);
		    parent->setOpen(true);
		    found = true;
		  }
	      }
	  }

      ++level;
    }

    // recursive delete all items not in new_items
    delete_recursive( _devices->firstChild(), new_items );

    _items = new_items;

    if (!_devices->selectedItem())
        selectionChanged(_devices->firstChild());
}


void USBViewer::selectionChanged(Q3ListViewItem *item)
{
  if (item)
    {
      Q_UINT32 busdev = item->text(1).toUInt();
      USBDevice *dev = USBDevice::find(busdev>>8, busdev&255);
      if (dev)
	{
	  _details->setText(dev->dump());
	  return;
	}
    }
  _details->clear();
}


