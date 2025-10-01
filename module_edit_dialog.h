#ifndef MODULE_EDIT_DIALOG_H
#define MODULE_EDIT_DIALOG_H

#include <QDialog>
#include <QLineEdit>
#include "xml_config.h"
#include <QTableWidget>

class ModuleEditDialog : public QDialog
{
    Q_OBJECT
public:
    explicit ModuleEditDialog(const Module &module, QWidget *parent = nullptr);
    ~ModuleEditDialog();

    Module getEditedModule() const;

    //static
    static Module editModule(const Module &module, QWidget *parent = nullptr);


private slots:
    void addParam();
    bool saveModule();

private:
    void setupDialog();
    void updateParamTable();
    bool validateRow(int row) const;
private:
    Module m_module;
    QTableWidget* m_paramTable;
    bool m_isEditing = false;


};

#endif // MODULE_EDIT_DIALOG_H
