/*****************************************************************

Copyright (c) 1996-2000 the kicker authors. See file AUTHORS.

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
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN
AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

******************************************************************/

#ifndef __panelbutton_h__
#define __panelbutton_h__

#include <algorithm>

#include <QAbstractButton>

#include <kpanelapplet.h>
#include <kpanelextension.h>
#include <k3urldrag.h>

#include "kickertip.h"
#include "utils.h"

class QColor;
class QDragEnterEvent;
class QDragLeaveEvent;
class QDropEvent;
class QEvent;
class QMenu;
class QMouseEvent;
class QPaintEvent;
class QPainter;
class QPixmap;
class QPoint;
class QResizeEvent;
class KConfigGroup;
class KShadowEngine;

/**
 * PanelButton is the base class for all buttons to be
 * placed in Kicker's panels. It inherits QButton, and
 * KickerTip::Client.
 */
class KDE_EXPORT PanelButton: public QAbstractButton, public KickerTip::Client
{
    Q_OBJECT

public:
    /**
     * Create a panel button
     * @param parent the parent widget
     */
    PanelButton( QWidget* parent );
    /**
     * Create a panel button
     * @param parent the parent widget
     * @param name the widget's name
     */
    KDE_CONSTRUCTOR_DEPRECATED PanelButton( QWidget* parent, const char* name );

    virtual ~PanelButton();

    /**
     * Prompts the button to save it's configuration. Subclass specific
     * settings should be saved in this method to the KConfigGroup passed in.
     */
    virtual void saveConfig(KConfigGroup&) const {}

    /**
     * Reimplement this to display a properties dialog for your button.
     */
    virtual void properties() {}

    /**
     * Reimplement this to give Kicker a hint for the width of the button
     * given a certain height.
     */
    virtual int widthForHeight(int height) const;

    /**
     * Reimplement this to give Kicker a hint for the height of the button
     * given a certain width.
     */
    virtual int heightForWidth(int width) const;

    /**
     * @return the button's current icon
     */
    virtual const QPixmap& labelIcon() const;

    /**
     * @return the button's zoom icon
     */
    virtual const QPixmap& zoomIcon() const;

     /**
     * @return true if this button is valid.
     */
    bool isValid() const;

    /**
     * Changes the title for the panel button.
     * @param t the button's title
     */
    void setTitle(const QString& t);

    /**
     * @return the title of the button.
     */
    QString title() const;

    /**
     * Set to true to draw an arrow on the button.
     */
    void setDrawArrow(bool drawArrow);

    /**
     * Used to set the icon for this panel button.
     * @param icon the path to the button's icon
     * TODO: Rename this to a better name.
     */
    void setIcon(const QString& icon);

    /**
     * @return the button's icon
     * TODO: Rename this to a better name.
     */
    QString icon() const;

    /**
     * @return whether this button has a text label or not
     */
    bool hasText() const;

    /**
     * Change the button's text label
     * @param text text for button's label
     */
    void setButtonText(const QString& text);

    /**
     * @return button's text label
     */
    QString buttonText() const;

    /**
     * Change the button's text label color
     * @param c the new text label color
     */
    void setTextColor(const QColor& c);

    /**
     * @return the button's text label color
     */
    QColor textColor() const;

    /**
     * Change the button's text scale
     * @param p font scale (in percent)
     */
    void setFontPercent(double p);

    /**
     * @return the button's text scale (in percent)
     */
    double fontPercent() const;

    /**
     * @return the orientation of the button
     */
    Qt::Orientation orientation() const;

    /**
     * @return the button's popup direction (read from parent KPanelApplet)
     */
    Plasma::Position popupDirection() const;

    /**
     * @return global position of the center of the button
     */
    QPoint center() const;

    /**
     * Update the contents of the button's KickerTip
     * @param data new KickerTip data
     */
    void updateTipData(KickerTip::Data& data);

Q_SIGNALS:
    /**
     * Emitted when the button's icon is changed.
     */
    void iconChanged();

    /**
     * Emitted to notify parent containers to save config
     */
    void requestSave();

    /**
     * Emitted when the button needs to be removed from it's container
     * @see KickerSettings::removeButtonsWhenBroken()
     */
    void removeme();

    /**
     * Emitted when the button may need to be removed, but that removal depends
     * on as-yet-uncertain future events and therefore ought to be hidden from
     * view, though not deleted quite yet.
     * @see KickerSettings::removeButtonsWhenBroken()
     */
    void hideme(bool hide);

    /**
     * Emitted when button initiates a drag
     */
    void dragme(const QPixmap);

    /**
     * Overloads dragme to support panel button's with a list of KUrl's ([url/servicemenu/browser]button)
     */
    void dragme(const KUrl::List, const QPixmap);

public Q_SLOTS:
    /**
     * Set to true to enable the button.
     */
    void setEnabled(bool enable);

    /**
     * Sets the orientation of the button (ie. which direction the icon will rotate).
     */
    void setOrientation(Qt::Orientation o);

    /**
     * Sets the direction to pop up the contents of the button.
     */
    void setPopupDirection(Plasma::Position d);

protected:
    /**
     * @return the default icon for the button
     */
    virtual QString defaultIcon() const { return "unknown"; };

    /**
     * Called right before drag occurs.
     */
    virtual void triggerDrag();

    /**
     * Emits a signal to drag the button. Reimplement this if, for example,
     * if you need the button to call dragme(KUrl::List, const QPixmap)
     * instead of dragme(const QPixmap)
     */
    virtual void startDrag();

    virtual void enterEvent(QEvent *);
    virtual void leaveEvent(QEvent *);
    virtual void dragEnterEvent(QDragEnterEvent *);
    virtual void dragLeaveEvent(QDragLeaveEvent *);
    virtual void dropEvent(QDropEvent *);
    virtual void mouseMoveEvent(QMouseEvent *);
    virtual void mousePressEvent(QMouseEvent *);
    virtual void mouseReleaseEvent(QMouseEvent *);
    virtual void resizeEvent(QResizeEvent*);
    virtual void paintEvent( QPaintEvent * );
    virtual void drawButton(QPainter *);
    virtual void drawButtonLabel(QPainter *);

    /**
     * @return the preferred icon size.
     */
    virtual int preferredIconSize(int proposed_size = -1) const;

    /**
     * @return the preferred dimensions for the button
     */
    virtual int preferredDimension(int panelDim) const;

    /**
     * if the button represents a local file, it tells PanelButton
     * what file that is and it starts to watch it. if the file is
     * deleted, it is disabled and then checked for one second later
     * to see if has returned (e.g. a reinstall occurred) by calling
     * checkForBackingFile(). if that returns false, then the button
     * is removed from kicker.
     * TODO: implement a heuristic that checks back in intervals for
     * the reappearance of the file and returns the button to the panel
     */
    virtual bool checkForBackingFile();

    /**
     * Set the file backing this button (See @ref checkForBackingFile()),
     * you shouldn't need to use this, currently it's only used in [url/service]button
     */
    void backedByFile(const QString& localFilePath);

    /**
     * Sets the button's arrow direction.
     * @param dir the arrow direction
     */
    void setArrowDirection(Plasma::Position dir);

    /**
     * Loads the icons for the button
     */
    void loadIcons();

    /**
     * (Re)Calculate icon sizes and return true if they have changed.
     */
    bool calculateIconSize();

    /**
     * Set if the panel button is valid.
     * @param valid true or false
     */
    void setIsValid(bool valid);

    /**
     * Set the icon Pixmap
     * @param icon Icon to load.
     */
    void setIconPixmap(const QPixmap &icon);

protected Q_SLOTS:
    /**
     * Called from KApplication when global icon settings have changed.
     * @param group the new group
     */
    void updateIcon(int group);

    /**
     * Called from KApplication when global settings have changed.
     * @param category the settings category, see KApplication::SettingsCategory
     */
    void updateSettings(int category);

    /**
     * Used for backedByFile, to check if the file backing this button
     * has been deleted.
     * @param path path to backing file
     */
    void checkForDeletion(const QString& path);

    /**
     * Called to prepare the button for removal from the Kicker
     */
    void scheduleForRemoval();

private:

    static KShadowEngine* s_textShadowEngine;

    class Private;
    Private* d;
};

/**
 * Base class for panelbuttons which popup a menu
 */
class KDE_EXPORT PanelPopupButton : public PanelButton
{
    Q_OBJECT

public:
   /**
    * Create a panel button that pops up a menu.
    * @param parent the parent widget
    * @param name the widget's name
    */
    PanelPopupButton(QWidget *parent=0, const char *name=0);

    virtual ~PanelPopupButton();

    /**
     * Sets the button's popup menu.
     * @param popup the menu to pop up
     */
    void setPopup(QMenu *popup);

    /**
     * @return the button's popup menu
     */
    QMenu *popup() const;

    bool eventFilter(QObject *, QEvent *);

protected:
    /**
     * Called each time the button is clicked and the popup
     * is displayed. Reimplement for dynamic popup menus.
     */
    virtual void initPopup() {};

    /**
     * Called before drag occurs. Reimplement to do any
     * necessary setup before the button is dragged.
     */
    virtual void triggerDrag();

    /**
     * Marks the menu as initialized.
     */
    void setInitialized(bool initialized);

protected Q_SLOTS:
    /**
     * Connected to the button's pressed() signal, this is
     * the code that actually displays the menu. Reimplement if
     * you need to take care of any tasks before the popup is
     * displayed (eg. KickerTip)
     */
    virtual void slotExecMenu();

private:
    class Private;
    Private* d;
};

#endif // __panelbutton_h__
