/*
  Copyright 2008 Google Inc.

  Licensed under the Apache License, Version 2.0 (the "License");
  you may not use this file except in compliance with the License.
  You may obtain a copy of the License at

       http://www.apache.org/licenses/LICENSE-2.0

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.
*/

#ifndef GGADGET_PACKAGE_H
#define GGADGET_PACKAGE_H

#include <Plasma/Package>
#include <Plasma/PackageStructure>
#include <Plasma/PackageMetadata>
class GadgetBrowserHost;
class GglPackage : public Plasma::PackageStructure {
  Q_OBJECT
 public:
  GglPackage(QObject *parent, const QVariantList &args);

  virtual ~GglPackage();
  virtual bool installPackage(const QString &archivePath,
                              const QString &packageRoot);
  virtual void createNewWidgetBrowser(QWidget *parent = 0);

  void gadgetBrowserClosed();

 private:
  GadgetBrowserHost *host_;
};

#endif
