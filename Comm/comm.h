#ifndef COMM_H
#define COMM_H
// 抽象类IComm
// 实现类SerialComm和NetworkComm
// 通信管理类CommManager
class IComm : public QObject
{
    Q_OBJECT
public:
    IComm();
};

#endif // COMM_H
