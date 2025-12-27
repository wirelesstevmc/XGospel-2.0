#ifndef PREFERENCES_DIALOG_H
#define PREFERENCES_DIALOG_H

#include <QDialog>
#include <QListWidget>
#include <QLineEdit>
#include <QPushButton>
#include <QComboBox>
#include "settings.h"

class PreferencesDialog : public QDialog {
    Q_OBJECT

public:
    explicit PreferencesDialog(QWidget *parent = nullptr);
    ~PreferencesDialog();

private slots:
    void onHostSelectionChanged();
    void onNewHost();
    void onDeleteHost();
    void onApply();
    void onOk();
    void onCancel();

private:
    void setupUI();
    void loadHosts();
    void updateHostFields();
    void clearHostFields();
    bool saveCurrentHost();

    // UI Elements
    QListWidget *m_host_list;

    QLineEdit *m_title_edit;
    QLineEdit *m_host_edit;
    QLineEdit *m_port_edit;
    QLineEdit *m_login_edit;
    QLineEdit *m_password_edit;
    QComboBox *m_codec_combo;

    QPushButton *m_new_btn;
    QPushButton *m_delete_btn;
    QPushButton *m_apply_btn;
    QPushButton *m_ok_btn;
    QPushButton *m_cancel_btn;

    // Data
    std::vector<Host> m_hosts;
    int m_current_host_index;
};

#endif // PREFERENCES_DIALOG_H
