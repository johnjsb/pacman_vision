#ifndef _LISTENER_GUI_H_
#define _LISTENER_GUI_H_

#include <QMainWindow>
#include <QPushButton>
#include <listener/listener_config.hpp>

namespace Ui {
class ListenerGui;
}

class ListenerGui : public QMainWindow
{
    Q_OBJECT

public:
    explicit ListenerGui(const pacv::ListenerConfig::Ptr conf, QWidget *parent = 0);
    ~ListenerGui();
    QWidget* getWidget() const;
    void setRunning(const bool run);
    QPushButton* getRunButt() const;

private slots:
    void on_RunningButt_clicked();
    void on_SwitchHandButt_clicked();

    void on_LRAButt_clicked(bool checked);

    void on_LLAButt_clicked(bool checked);

    void on_LRHButt_clicked(bool checked);

    void on_LLHButt_clicked(bool checked);

    void on_RRAButt_clicked(bool checked);

    void on_RLAButt_clicked(bool checked);

    void on_RRHButt_clicked(bool checked);

    void on_RLHButt_clicked(bool checked);

private:
    void init();
    bool right;
    Ui::ListenerGui *ui;
    pacv::ListenerConfig::Ptr config;
    bool running;
};

#endif
