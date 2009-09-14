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

#ifndef GGADGET_GGL_EXTENSIONS_H
#define GGADGET_GGL_EXTENSIONS_H

// Extensions that should be loaded by ggl system
static const char *kGlobalExtensions[] = {
  "default-framework",
  "libxml2-xml-parser",
  "dbus-script-class",
  "default-options",
  "qtwebkit-browser-element",
  "qt-system-framework",
  "qt-edit-element",
//  "phonon-audio-framework",
  "gst-audio-framework",
  "gst-video-element",
  "linux-system-framework",
  "qt-xml-http-request",
  "google-gadget-manager",
  "analytics-usage-collector",
  "smjs-script-runtime",
  "qt-script-runtime",
  NULL
};

#endif
