=begin
/*
Copyright (c) 2007 Zack Rusin <zack@kde.org>

Translated to Ruby by Richard Dale                                    *

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
 */
=end

require 'plasma_applet'

module RubyWebapplet

class Main < PlasmaScripting::Applet

  slots 'dataUpdated(QString,Plasma::DataEngine::Data)',
        'load(QUrl)',
        'setHtml(QByteArray)',
        'loadHtml(QUrl)',
        'loadFinished(bool)'

  def initialize(parent, args = nil)
    super
  end

  def init
    resize(600, 400)

    @page = Plasma::WebView.new(self)
    @page.page = Qt::WebPage.new(@page)
    @page.page.linkDelegationPolicy = Qt::WebPage::DelegateAllLinks
    @page.page.settings.setAttribute(Qt::WebSettings::LinksIncludedInFocusChain, true)

    connect(@page, SIGNAL('loadFinished(bool)'), self, SLOT('loadFinished(bool)'))
    connect(@page.page, SIGNAL('linkClicked(QUrl)'), self, SLOT('load(QUrl)'))

    @page.mainFrame.setScrollBarPolicy(Qt::Horizontal, Qt::ScrollBarAlwaysOff)
    @page.mainFrame.setScrollBarPolicy(Qt::Vertical, Qt::ScrollBarAlwaysOff)

    @page.url = Qt::Url.new("http://dot.kde.org/")
  end

  def paintInterface(p, option, rect)
  end

  def load(url)
    puts "Loading #{url.toString}"
    @page.url = url
  end

  def view
    @page
  end

  def loadFinished(success)
    puts "page loaded #{@page.page.currentFrame.url.toString}"
  end

  def constraintsEvent(constraints)
    if constraints.to_i & Plasma::SizeConstraint.to_i
      @page.resize(size())
    end
  end

  def setHtml(html, baseUrl = Qt::Url.new)
    puts "loading #{baseUrl.toString}"
    @page.mainFrame.setHtml(html, baseUrl)
  end

  def loadHtml(url = Qt::Url.new)
    puts "loading #{url.toString}"
    @page.mainFrame.load(url)
  end
end

end
