#include "qkconnectclientthread.h"
#include "qkconnectsocket.h"
#include "qkconnect_global.h"
#include "qkconnectserver.h"

#include <QDebug>
#include <QHostAddress>

QkConnectClientThread::QkConnectClientThread(Broker *broker, int socketDescriptor, QObject *parent) :
    QThread(parent)
{
    _socketDescriptor = socketDescriptor;
    _socket = 0;
    _broker = broker;
}

QkConnectSocket* QkConnectClientThread::socket()
{
    return _socket;
}

void QkConnectClientThread::run()
{
    QkConnectSocket socket;
    _socket = &socket;

    if(!_socket->setSocketDescriptor(_socketDescriptor))
    {
        emit message(QKCONNECT_MESSAGE_ERROR, _socket->errorString());
        return;
    }


    emit clientConnected(_socketDescriptor);
    emit message(QKCONNECT_MESSAGE_INFO,
                 tr("Hey: ") + _socket->peerAddress().toString() + QString().sprintf(" (%d)", _socketDescriptor));

    connect(_broker, SIGNAL(dataToClient(QByteArray)), _socket, SLOT(sendData(QByteArray)));
    connect(_socket, SIGNAL(dataIn(QByteArray)), this, SLOT(_slotDataIn(QByteArray)), Qt::DirectConnection);
    connect(_socket, SIGNAL(disconnected()), this, SLOT(_slotDisconnected()), Qt::DirectConnection);

    exec();
}

void QkConnectClientThread::sendData(QByteArray data)
{
    qDebug() << __PRETTY_FUNCTION__;
    if(_socket->isOpen() && !data.isEmpty())
        _socket->write(data);
}

void QkConnectClientThread::_slotDataIn(QByteArray data)
{
    emit dataIn(_socketDescriptor, data);
}

void QkConnectClientThread::_slotDisconnected()
{
    emit clientDisconnected(_socketDescriptor);
    emit message(QKCONNECT_MESSAGE_INFO,
                 tr("Bye: ") + _socket->peerAddress().toString() + QString().sprintf(" (%d)", _socketDescriptor));
    quit();
}