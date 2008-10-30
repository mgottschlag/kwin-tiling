=begin
 *   Copyright 2008 by Richard Dale <richard.j.dale@gmail.com>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU Library General Public License as
 *   published by the Free Software Foundation; either version 2, or
 *   (at your option) any later version.
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
=end

require 'plasma_applet'

module PlasmaScriptengineRuby
  class PackageRuboid < Plasma::PackageStructure
    def initialize(parent, args)
      super(parent, "Ruboid")
      addDirectoryDefinition("images", "images", KDE::i18n("Images"))
      mimetypes = []
      mimetypes << "image/svg+xml" << "image/png" << "image/jpeg";
      setMimetypes("images", mimetypes)

      addDirectoryDefinition("config", "config/", KDE::i18n("Configuration Definitions"))
      mimetypes = []
      mimetypes << "text/xml";
      setMimetypes("config", mimetypes)
      setMimetypes("configui", mimetypes)

      addDirectoryDefinition("ui", "ui", KDE::i18n("Executable Scripts"))
      setMimetypes("ui", mimetypes)

      addDirectoryDefinition("scripts", "code", KDE::i18n("Executable Scripts"))
      mimetypes = []
      mimetypes << "text/*"
      setMimetypes("scripts", mimetypes)

      addFileDefinition("mainconfiggui", "ui/config.ui", KDE::i18n("Main Config UI File"))
      addFileDefinition("mainconfigxml", "config/main.xml", KDE::i18n("Configuration XML file"))
      addFileDefinition("mainscript", "code/main.rb", KDE::i18n("Main Script File"))
      setRequired("mainscript", true)
    end
  end
end