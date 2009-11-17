/***************************************************************************
 *   Copyright 2006 by Aaron Seigo <aseigo@kde.org>                        *
 *   Copyright 2008 by Davide Bettio <davide.bettio@kdemail.net>           *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA .        *
 ***************************************************************************/

#include "interface.h"

#include <QAction>
#include <QApplication>
#include <QClipboard>
#include <QDesktopWidget>
#include <QGraphicsView>
#include <QHBoxLayout>
#include <QHideEvent>
#include <QLabel>
#include <QShortcut>
#include <QToolButton>
#include <QVBoxLayout>

#include <KActionCollection>
#include <KHistoryComboBox>
#include <KCompletion>
#include <KCompletionBox>
#include <KDebug>
#include <KDialog>
#include <KLineEdit>
#include <KLocale>
#include <KGlobalSettings>
#include <KPushButton>
#include <KTitleWidget>

#include <Plasma/AbstractRunner>
#include <Plasma/RunnerManager>
#include <Plasma/Theme>
#include <Plasma/Svg>

#include "krunnersettings.h"
#include "interfaces/default/resultscene.h"
#include "interfaces/default/resultitem.h"
#include "interfaces/default/krunnertabfilter.h"
#include "interfaces/default/resultsview.h"

static const int MIN_WIDTH = 420;

Interface::Interface(Plasma::RunnerManager *runnerManager, QWidget *parent)
    : KRunnerDialog(runnerManager, parent),
      m_delayedRun(false),
      m_running(false),
      m_queryRunning(false)
{
    m_hideResultsTimer.setSingleShot(true);
    connect(&m_hideResultsTimer, SIGNAL(timeout()), this, SLOT(hideResultsArea()));

    QWidget *w = mainWidget();
    m_layout = new QVBoxLayout(w);
    m_layout->setMargin(0);

    m_buttonContainer = new QWidget(w);
    QHBoxLayout *bottomLayout = new QHBoxLayout(m_buttonContainer);
    bottomLayout->setMargin(0);

    m_configButton = new QToolButton(m_buttonContainer);
    m_configButton->setText(i18n("Settings"));
    m_configButton->setToolTip(i18n("Settings"));
    m_configButton->setIcon(m_iconSvg->pixmap("configure"));
    connect(m_configButton, SIGNAL(clicked()), SLOT(showConfigDialog()));
    bottomLayout->addWidget( m_configButton );

    /*
    KPushButton *m_optionsButton = new KPushButton(KStandardGuiItem::configure(), m_buttonContainer);
    m_optionsButton->setDefault(false);
    m_optionsButton->setAutoDefault(false);
    m_optionsButton->setText(i18n("Show Options"));
    m_optionsButton->setEnabled(false);
    m_optionsButton->setCheckable(true);
    connect(m_optionsButton, SIGNAL(toggled(bool)), SLOT(showOptions(bool)));
    bottomLayout->addWidget( m_optionsButton );
    */

    m_activityButton = new QToolButton(m_buttonContainer);
//    m_activityButton->setDefault(false);
//    m_activityButton->setAutoDefault(false);
    m_activityButton->setText(i18n("Show System Activity"));
    m_activityButton->setToolTip(i18n("Show System Activity"));
    m_activityButton->setIcon(m_iconSvg->pixmap("status"));
    connect(m_activityButton, SIGNAL(clicked()), qApp, SLOT(showTaskManager()));
    connect(m_activityButton, SIGNAL(clicked()), this, SLOT(close()));
    bottomLayout->addWidget(m_activityButton);
    //bottomLayout->addStretch(10);

    m_helpButton = new QToolButton(m_buttonContainer);
    m_helpButton->setText(i18n("Help"));
    m_helpButton->setToolTip(i18n("Information on using this application"));
    m_helpButton->setIcon(m_iconSvg->pixmap("help"));
    connect(m_helpButton, SIGNAL(clicked(bool)), SLOT(showHelp()));
    connect(m_helpButton, SIGNAL(clicked(bool)), SLOT(configCompleted()));
    bottomLayout->addWidget(m_helpButton);

    QSpacerItem* closeButtonSpacer = new QSpacerItem(0,0,QSizePolicy::MinimumExpanding,QSizePolicy::Fixed);
    bottomLayout->addSpacerItem(closeButtonSpacer);

    m_closeButton = new QToolButton(m_buttonContainer);
    KGuiItem guiItem = KStandardGuiItem::close();
    m_closeButton->setText(guiItem.text());
    m_closeButton->setToolTip(guiItem.text().remove('&'));
    m_closeButton->setIcon(m_iconSvg->pixmap("close"));
//    m_closeButton->setDefault(false);
//    m_closeButton->setAutoDefault(false);
    connect(m_closeButton, SIGNAL(clicked(bool)), SLOT(close()));
    bottomLayout->addWidget(m_closeButton);

    m_layout->addWidget(m_buttonContainer);

    m_searchTerm = new KHistoryComboBox(false, w);
    m_searchTerm->setPalette(QApplication::palette());
    m_searchTerm->setDuplicatesEnabled(false);

    KLineEdit *lineEdit = new KLineEdit(m_searchTerm);
    QAction *focusEdit = new QAction(this);
    focusEdit->setShortcut(Qt::Key_F6);

    // in therory, the widget should detect the direction from the content
    // but this is not available in Qt4.4/KDE 4.2, so the best default for this widget
    // is LTR: as it's more or less a "command line interface"
    // FIXME remove this code when KLineEdit has automatic direction detection of the "paragraph"
    m_searchTerm->setLayoutDirection( Qt::LeftToRight );

    connect(focusEdit, SIGNAL(triggered(bool)), lineEdit, SLOT(setFocus()));
    addAction(focusEdit);

    // the order of these next few lines if very important.
    // QComboBox::setLineEdit sets the autoComplete flag on the lineedit,
    // and KComboBox::setAutoComplete resets the autocomplete mode! ugh!
    m_searchTerm->setLineEdit(lineEdit);

    m_completion = new KCompletion();
    lineEdit->setCompletionObject(m_completion);
    lineEdit->setCompletionMode(static_cast<KGlobalSettings::Completion>(KRunnerSettings::queryTextCompletionMode()));
    lineEdit->setClearButtonShown(true);
    QStringList pastQueryItems = KRunnerSettings::pastQueries();
    m_searchTerm->setHistoryItems(pastQueryItems);
    m_searchTerm->setSizeAdjustPolicy(QComboBox::AdjustToMinimumContentsLengthWithIcon);
    m_completion->insertItems(pastQueryItems);
    bottomLayout->insertWidget(2, m_searchTerm, 10);

    m_resultsContainer = new QWidget(w);
    QVBoxLayout* resultsLayout = new QVBoxLayout(m_resultsContainer);
    resultsLayout->setMargin(0);

    m_dividerLine = new QWidget(m_resultsContainer);
    m_dividerLine->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Minimum);
    m_dividerLine->setFixedHeight(1);
    m_dividerLine->setAutoFillBackground(true);
    resultsLayout->addWidget(m_dividerLine);

    m_resultsView = new ResultsView(m_resultsContainer);

    //kDebug() << "size:" << m_resultsView->size() << m_resultsView->minimumSize();
    m_resultsScene = new ResultScene(runnerManager, m_searchTerm, this);
    m_resultsView->setScene(m_resultsScene);
    m_resultsView->setMinimumSize(m_resultsScene->minimumSizeHint());

    connect(m_resultsScene, SIGNAL(matchCountChanged(int)), this, SLOT(matchCountChanged(int)));
    connect(m_resultsScene, SIGNAL(itemActivated(ResultItem *)), this, SLOT(run(ResultItem *)));
    connect(m_resultsScene, SIGNAL(ensureVisibility(QGraphicsItem *)), this, SLOT(ensureVisibility(QGraphicsItem *)));

    resultsLayout->addWidget(m_resultsView);

    m_layout->addWidget(m_resultsContainer);

    connect(lineEdit, SIGNAL(userTextChanged(QString)), this, SLOT(queryTextEdited(QString)));
    connect(m_searchTerm, SIGNAL(returnPressed()), this, SLOT(runDefaultResultItem()));

    KrunnerTabFilter *krunnerTabFilter = new KrunnerTabFilter(m_resultsScene, lineEdit, this);
    m_searchTerm->installEventFilter(krunnerTabFilter);

    themeUpdated();
    connect(Plasma::Theme::defaultTheme(), SIGNAL(themeChanged()), this, SLOT(themeUpdated()));

    new QShortcut(QKeySequence(Qt::Key_Escape), this, SLOT(close()));

    m_layout->setAlignment(Qt::AlignTop);

    setTabOrder(0, m_configButton);
    setTabOrder(m_configButton, m_activityButton);
    setTabOrder(m_activityButton, m_searchTerm);
    setTabOrder(m_searchTerm, m_resultsView);
    setTabOrder(m_resultsView, m_helpButton);
    setTabOrder(m_helpButton, m_closeButton);

    //kDebug() << "size:" << m_resultsView->size() << m_resultsView->minimumSize() << minimumSizeHint();

    // we restore the original size, which will set the results view back to its
    // normal size, then we hide the results view and resize the dialog

    setMinimumSize(QSize(MIN_WIDTH , 0));
    adjustSize();

    // we load the last used size; the saved value is the size of the dialog when the
    // results are visible;

    if (KGlobal::config()->hasGroup("Interface")) {
        KConfigGroup interfaceConfig(KGlobal::config(), "Interface");
        restoreDialogSize(interfaceConfig);
    }

    m_defaultSize = size();
    m_resultsContainer->hide();

    QTimer::singleShot(0, this, SLOT(resetInterface()));
}

void Interface::setConfigWidget(QWidget *w)
{
    m_resultsView->hide();
    m_searchTerm->setEnabled(false);
    m_layout->addWidget(w);
    resize(m_defaultSize);
    connect(w, SIGNAL(destroyed(QObject*)), this, SLOT(configWidgetDestroyed()));
}

void Interface::configWidgetDestroyed()
{
    QTimer::singleShot(0, this, SLOT(cleanupAfterConfigWidget()));
}

void Interface::cleanupAfterConfigWidget()
{
    m_resultsView->show();
    resize(qMax(minimumSizeHint().width(), m_defaultSize.width()), minimumSizeHint().height());
    m_searchTerm->setEnabled(true);
    m_searchTerm->setFocus();
}

void Interface::resizeEvent(QResizeEvent *event)
{
    // We set m_defaultSize only when the event is spontaneous, i.e. when the user resizes the window
    // We always update the width, but we update the height only if the resultsContainer is visible.

    if (event->spontaneous() || isManualResizing()) {
        m_defaultSize.setWidth(width());
        if (m_resultsContainer->isVisible()) {
            m_defaultSize.setHeight(height());
        }
    }

    Plasma::Theme *theme = Plasma::Theme::defaultTheme();
    int gradientWidth = contentsRect().width() - KDialog::marginHint()*2;
    QLinearGradient gr(0, 0, gradientWidth, 0);
    gr.setColorAt(0, Qt::transparent);
    gr.setColorAt(.35, theme->color(Plasma::Theme::TextColor));
    gr.setColorAt(.65, theme->color(Plasma::Theme::TextColor));
    gr.setColorAt(1, Qt::transparent);
    {
        QPalette p = palette();
        p.setBrush(QPalette::Background, gr);
        m_dividerLine->setPalette(p);
    }

    m_resultsScene->resize(m_resultsView->width(), qMax(m_resultsView->height(), int(m_resultsScene->height())));

    KRunnerDialog::resizeEvent(event);
}

Interface::~Interface()
{
    KRunnerSettings::setPastQueries(m_searchTerm->historyItems());
    KRunnerSettings::setQueryTextCompletionMode(m_searchTerm->completionMode());
    KRunnerSettings::self()->writeConfig();

    // Before saving the size we resize to the default size, with the results container shown.
    resize(m_defaultSize);
    KConfigGroup interfaceConfig(KGlobal::config(), "Interface");
    saveDialogSize(interfaceConfig);
    KGlobal::config()->sync();
}

void Interface::themeUpdated()
{
    Plasma::Theme *theme = Plasma::Theme::defaultTheme();
    QColor buttonBgColor = theme->color(Plasma::Theme::BackgroundColor);
    QString buttonStyleSheet = QString("QToolButton { border: 1px solid %4; border-radius: 4px; padding: 2px;"
                                       " background-color: rgba(%1, %2, %3, %5); }")
                                      .arg(buttonBgColor.red())
                                      .arg(buttonBgColor.green())
                                      .arg(buttonBgColor.blue())
                                      .arg(theme->color(Plasma::Theme::BackgroundColor).name(), "50%");
    buttonBgColor = theme->color(Plasma::Theme::TextColor);
    buttonStyleSheet += QString("QToolButton:hover { border: 2px solid %1; }")
                               .arg(theme->color(Plasma::Theme::HighlightColor).name());
    buttonStyleSheet += QString("QToolButton:focus { border: 2px solid %1; }")
                               .arg(theme->color(Plasma::Theme::HighlightColor).name());
    m_configButton->setStyleSheet(buttonStyleSheet);
    m_activityButton->setStyleSheet(buttonStyleSheet);
    m_helpButton->setStyleSheet(buttonStyleSheet);
    m_closeButton->setStyleSheet(buttonStyleSheet);
    //kDebug() << "stylesheet is" << buttonStyleSheet;

    //reset the icons
    m_configButton->setIcon(m_iconSvg->pixmap("configure"));
    m_activityButton->setIcon(m_iconSvg->pixmap("status"));
    m_closeButton->setIcon(m_iconSvg->pixmap("close"));
}

void Interface::clearHistory()
{
    m_searchTerm->clearHistory();
    KRunnerSettings::setPastQueries(m_searchTerm->historyItems());
}

void Interface::display(const QString &term)
{
    m_searchTerm->setFocus();

    if (!term.isEmpty() || !isVisible()) {
        resetInterface();
    }

    positionOnScreen();

    if (!term.isEmpty()) {
        m_searchTerm->setItemText(0, term);
    }
}

void Interface::setWidgetPalettes()
{
    // a nice palette to use with the widgets
    QPalette widgetPalette = palette();
    QColor bgColor = widgetPalette.color( QPalette::Active,
                                                QPalette::Base );
    bgColor.setAlpha( 200 );
    widgetPalette.setColor( QPalette::Base, bgColor );

    m_searchTerm->setPalette( widgetPalette );
}

void Interface::resetInterface()
{
    setStaticQueryMode(false);
    m_delayedRun = false;
    m_searchTerm->setCurrentItem(QString(), true, 0);
    resetResultsArea();
    m_resultsScene->clearQuery();
    resize(qMax(minimumSizeHint().width(), m_defaultSize.width()), minimumSizeHint().height());
}

void Interface::showHelp()
{
    QMap<QString, Plasma::QueryMatch> matches;

    foreach (Plasma::AbstractRunner *runner, m_runnerManager->runners()) {
        int count = 0;
        QIcon icon(runner->icon());
        if (icon.isNull()) {
            icon = KIcon("system-run");
        }

        foreach (const Plasma::RunnerSyntax &syntax, runner->syntaxes()) {
            Plasma::QueryMatch match(0);
            match.setType(Plasma::QueryMatch::InformationalMatch);
            match.setIcon(icon);
            match.setText(syntax.exampleQueriesWithTermDescription().join(", "));
            match.setSubtext(syntax.description() + "\n" +
                             i18n("(From %1, %2)", runner->name(), runner->description()));
            match.setData(syntax.exampleQueries().first());
            matches.insert(runner->name() + QString::number(++count), match);
        }
    }

    m_resultsScene->setQueryMatches(matches.values());
}

void Interface::ensureVisibility(QGraphicsItem* item)
{
    m_resultsScene->setItemsAcceptHoverEvents(false);
    m_resultsView->ensureVisible(item,0,0);
    m_resultsScene->setItemsAcceptHoverEvents(true);
}

void Interface::setStaticQueryMode(bool staticQuery)
{
     // don't show the search and other control buttons in the case of a static query
  //   m_buttonContainer->setVisible(!staticQuery);
    bool visible = !staticQuery;
    m_configButton->setVisible(visible);
    m_activityButton->setVisible(visible);
    m_helpButton->setVisible(visible);
    m_searchTerm->setVisible(visible);
}

void Interface::hideEvent(QHideEvent *e)
{
    KRunnerDialog::hideEvent(e);

    if (!m_running) {
        resetInterface();
    } else {
        m_delayedRun = false;
        resetResultsArea();
        adjustSize();
    }
    e->accept();
}

void Interface::run(ResultItem *item)
{
    if (!item || item->group() < Plasma::QueryMatch::PossibleMatch) {
        m_delayedRun = true;
        return;
    }

    kDebug() << item->name() << item->id();
    m_delayedRun = false;

    if (item->group() == Plasma::QueryMatch::InformationalMatch) {
        QString info = item->data();
        int editPos = info.length();

        if (!info.isEmpty()) {
            if (item->isQueryPrototype()) {
                // lame way of checking to see if this is a Help Button generated match!
                int index = info.indexOf(":q:");

                if (index != -1) {
                    editPos = index;
                    info.replace(":q:", "");
                }
            }

            m_searchTerm->setItemText(0, info);
            m_searchTerm->lineEdit()->setCursorPosition(editPos);
            m_searchTerm->setCurrentIndex(0);
            QApplication::clipboard()->setText(info);
        }
        return;
    }

    m_running = true;
    close();
    m_resultsScene->run(item);
    m_running = false;

    //TODO: check if run is succesful before adding the term to history
    m_searchTerm->addToHistory(m_searchTerm->currentText().trimmed());
    resetInterface();
}

void Interface::runDefaultResultItem()
{
    if (m_queryRunning) {
        m_delayedRun = true;
    } else {
        run(m_resultsScene->defaultResultItem());
    }
}

void Interface::queryTextEdited(const QString &query)
{
    m_delayedRun = false;

    if (query.isEmpty()) {
        resetInterface();
        m_queryRunning = false;
    } else {
        m_queryRunning = m_resultsScene->launchQuery(query) || m_queryRunning; //lazy OR?
    }
}

void Interface::matchCountChanged(int count)
{
    m_queryRunning = false;
    bool show = count > 0;
    m_hideResultsTimer.stop();

    if (show && m_delayedRun) {
        kDebug() << "delayed run with" << count << "items";
        runDefaultResultItem();
        return;
    }

    if (m_resultsView->isVisible() == show) {
        return;
    }

    if (show) {
        //kDebug() << "showing!" << minimumSizeHint();
        m_resultsContainer->show();

        // Next 2 lines are a workaround to allow arrow
        // keys navigation in krunner's result list.
        // Patch submited in bugreport #211578
        QEvent event(QEvent::WindowActivate);
        QApplication::sendEvent(m_resultsView, &event);

        resize(m_defaultSize);
        m_resultsScene->resize(m_resultsView->width(), qMax(m_resultsView->height(), int(m_resultsScene->height())));
    } else {
        //kDebug() << "hiding ... eventually";
        m_delayedRun = false;
        m_hideResultsTimer.start(1000);
    }
}

void Interface::hideResultsArea()
{
    m_searchTerm->setFocus();

    resetResultsArea();

    resize(qMax(minimumSizeHint().width(), m_defaultSize.width()), minimumSizeHint().height());
}

void Interface::resetResultsArea()
{
    setMinimumSize(QSize(MIN_WIDTH,0));
    m_resultsContainer->hide();

    //This is a workaround for some Qt bug which is not fully understood; it seems that
    //adding a qgv to a layout and then hiding it gives some issues with resizing
    //Calling updateGeometry should not be necessary, but it probably triggers some updates which do the trick

    mainWidget()->updateGeometry();
}

#include "interface.moc"
