/***************************************************************************
 *   Copyright (C) 2001 by Matthias Hoelzer-Kluepfel <mhk@caldera.de>      *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/




#include <qlayout.h>
#include <qgroupbox.h>
#include <qsplitter.h>
#include <qlistview.h>
#include <qtextview.h>
#include <qheader.h>
#include <qtimer.h>

#include <kgenericfactory.h>
#include <kaboutdata.h>
#include <kdialog.h>

#include "usbdevices.h"
#include "kcmusb.moc"

typedef KGenericFactory<USBViewer, QWidget > USBFactory;
K_EXPORT_COMPONENT_FACTORY (kcm_usb, USBFactory("kcmusb") )

USBViewer::USBViewer(QWidget *parent, const char *name, const QStringList &)
  : KCModule(USBFactory::instance(), parent, name)
{
  setButtons(Help);

  QVBoxLayout *vbox = new QVBoxLayout(this, 0, KDialog::spacingHint());
  QGroupBox *gbox = new QGroupBox(i18n("USB Devices"), this);
  gbox->setColumnLayout( 0, Qt::Horizontal );
  vbox->addWidget(gbox);

  QVBoxLayout *vvbox = new QVBoxLayout(gbox->layout(), KDialog::spacingHint());

  QSplitter *splitter = new QSplitter(gbox);
  vvbox->addWidget(splitter);

  _devices = new QListView(splitter);
  _devices->addColumn(i18n("Device"));
  _devices->setRootIsDecorated(true);
  _devices->header()->hide();
  _devices->setMinimumWidth(200);
  _devices->setColumnWidthMode(0, QListView::Maximum);

  QValueList<int> sizes;
  sizes.append(200);
  splitter->setSizes(sizes);

  _details = new QTextView(splitter);

  splitter->setResizeMode(_devices, QSplitter::KeepSize);

  QTimer *refreshTimer = new QTimer(this);
  // 1 sec seems to be a good compromise between latency and polling load.
  refreshTimer->start(1000);

  connect(refreshTimer, SIGNAL(timeout()), SLOT(refresh()));
  connect(_devices, SIGNAL(selectionChanged(QListViewItem*)),
	  this, SLOT(selectionChanged(QListViewItem*)));

  KAboutData *about =
  new KAboutData(I18N_NOOP("kcmusb"), I18N_NOOP("KDE USB Viewer"),
                0, 0, KAboutData::License_GPL,
                I18N_NOOP("(c) 2001 Matthias Hoelzer-Kluepfel"));

  about->addAuthor("Matthias Hoelzer-Kluepfel", 0, "mhk@kde.org");
  about->addCredit("Leo Savernik", "Live Monitoring of USB Bus", "l.savernik@aon.at");
  setAboutData( about );

  load();
}


USBViewer::~USBViewer()
{
}


void USBViewer::load()
{
  _items.clear();
  _devices->clear();

  refresh();
}

void USBViewer::refresh()
{
  QIntDict<QListViewItem> new_items;

  if (!USBDevice::parse("/proc/bus/usb/devices"))
    USBDevice::parse("/proc/bus/usb/devices_please-use-sysfs-instead");

  int level = 0;
  bool found = true;

  while (found)
    {
      found = false;

      QPtrListIterator<USBDevice> it(USBDevice::devices());
      for ( ; it.current(); ++it)
	if (it.current()->level() == level)
	  {
	    if (level == 0)
	      {
		QListViewItem *item = _items.find(it.current()->bus()*256+it.current()->device());
		if (!item) {
		    item = new QListViewItem(_devices,
				it.current()->product(),
				QString("%1").arg(it.current()->bus()),
				QString("%1").arg(it.current()->device()) );
		}
		new_items.insert(it.current()->bus()*256+it.current()->device(),
				item);
		found = true;
	      }
	    else
	      {
		QListViewItem *parent = new_items.find(it.current()->bus()*256+it.current()->parent());
		if (parent)
		  {
		    QListViewItem *item = _items.find(it.current()->bus()*256+it.current()->device());

		    if (!item) {
		        item = new QListViewItem(parent,
				    it.current()->product(),
				    QString("%1").arg(it.current()->bus()),
				    QString("%1").arg(it.current()->device()) );
		    }
		    new_items.insert(it.current()->bus()*256+it.current()->device(),
				item);
		    parent->setOpen(true);
		    found = true;
		  }
	      }
	  }

      ++level;
    }

    // delete all items not in new_items
    {
        QIntDictIterator<QListViewItem> it(_items);
        for (; it.current(); ++it) {
            if (!new_items.find(it.currentKey()))
	        delete it.current();
        }
    }

    _items = new_items;

    if (!_devices->selectedItem())
        selectionChanged(_devices->firstChild());
}


void USBViewer::selectionChanged(QListViewItem *item)
{
  if (item)
    {
      USBDevice *dev = USBDevice::find(item->text(1).toInt(),
		      item->text(2).toInt());
      if (dev)
	{
	  _details->setText(dev->dump());
	  return;
	}
    }
  _details->setText("");
}


void USBViewer::save()
{
}


void USBViewer::defaults()
{
}


QString USBViewer::quickHelp() const
{
  return i18n("<h1>USB Devices</h1> This module allows you to see"
     " the devices attached to your USB bus(es).");
}

