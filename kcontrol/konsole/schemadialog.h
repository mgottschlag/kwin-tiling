/****************************************************************************
** Form interface generated from reading ui file './schemadialog.ui'
**
** Created: Fri Apr 20 16:07:57 2001
**      by:  The User Interface Compiler (uic)
**
** WARNING! All changes made in this file will be lost!
****************************************************************************/
#ifndef SCHEMADIALOG_H
#define SCHEMADIALOG_H

#include <qvariant.h>
#include <qwidget.h>
class QVBoxLayout; 
class QHBoxLayout; 
class QGridLayout; 
class KColorButton;
class QCheckBox;
class QComboBox;
class QGroupBox;
class QLabel;
class QLineEdit;
class QListBox;
class QListBoxItem;
class QPushButton;
class QSlider;
class QToolButton;

class SchemaDialog : public QWidget
{ 
    Q_OBJECT

public:
    SchemaDialog( QWidget* parent = 0, const char* name = 0, WFlags fl = 0 );
    ~SchemaDialog();

    QGroupBox* GroupBox2;
    QPushButton* removeButton;
    QPushButton* saveButton;
    QListBox* schemaList;
    QCheckBox* defaultSchemaCB;
    QGroupBox* GroupBox1;
    QComboBox* colorCombo;
    QLabel* TextLabel8;
    QCheckBox* boldCheck;
    QCheckBox* transparentCheck;
    KColorButton* colorButton;
    QComboBox* typeCombo;
    QLabel* TextLabel1_2;
    QLabel* TextLabel1;
    QLineEdit* titleLine;
    QGroupBox* GroupBox13;
    QLineEdit* backgndLine;
    QToolButton* imageBrowse;
    QComboBox* modeCombo;
    QLabel* TextLabel11;
    QLabel* TextLabel6;
    QSlider* shadeSlide;
    QLabel* TextLabel5;
    QLabel* TextLabel3;
    KColorButton* shadeColor;
    QLabel* previewPixmap;
    QCheckBox* transparencyCheck;

protected:
    QGridLayout* SchemaDialogLayout;
    QGridLayout* GroupBox2Layout;
    QGridLayout* GroupBox1Layout;
    QGridLayout* GroupBox13Layout;
};

#endif // SCHEMADIALOG_H
