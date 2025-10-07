#ifndef COMM_MANAGER_H
#define COMM_MANAGER_H

#include <QObject>
#include <QSerialPort>
#include <QTcpSocket>

class CommManager : public QObject
{
    Q_OBJECT
public:
    explicit CommManager(QObject *parent = nullptr);
    ~CommManager();

};

#endif // COMM_MANAGER_H
