/*
Dwarf Therapist
Copyright (c) 2009 Trey Stout (chmod)

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

#ifndef SCRIPT_DIALOG_H
#define SCRIPT_DIALOG_H

#include <QDialog>
#include <QKeyEvent>

namespace Ui {
class ScriptDialog;
}

class ScriptDialog : public QDialog {
    Q_OBJECT
public:
    ScriptDialog(QWidget *parent = 0);
    virtual ~ScriptDialog();
    bool event(QEvent *evt);

public slots:
    //! clear the script editing box
    void clear_script();
    void load_script(QString name, QString script);    

private:
    Ui::ScriptDialog *ui;
    QString m_name;
    void reposition_horiz_splitters();
    void reposition_cursors();
    bool script_is_valid();

private slots:
    void close_pressed();
    void apply_pressed();
    void save_pressed();

protected:
    void closeEvent(QCloseEvent *){close_pressed();}
    void keyPressEvent(QKeyEvent *e){
        if(e->key()==Qt::Key_Escape)
            close_pressed();
    }

signals:
    //! tell the app to run the script currently being edited
    void test_script( const QString &script_body);
    //! tell the app that the user has just saved a script
    void scripts_changed();
};

#endif
