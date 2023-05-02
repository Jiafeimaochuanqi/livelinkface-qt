#include <QCoreApplication>
#include <QtNetwork/QUdpSocket>
#include "livelinkface.h"

#define UDP_PORT 11111
int main(int argc, char *argv[])
{
    qDebug() << sizeof(float);
    QUdpSocket udpSocket;
    udpSocket.bind(QHostAddress::Any, UDP_PORT);
    //    while (true) {
    //        if (udpSocket.hasPendingDatagrams()) {
    //            qDebug() <<"-------------------";
    //            QByteArray datagram;
    //            datagram.resize(udpSocket.pendingDatagramSize());
    //            QHostAddress sender;
    //            quint16 senderPort;

    //            udpSocket.readDatagram(datagram.data(), datagram.size(),
    //                                   &sender, &senderPort);

    //            bool success;
    //            PyLiveLinkFace liveLinkFace;
    //            success= liveLinkFace.decode(datagram);

    //            if (success) {
    //                // get the blendshape value for the HeadPitch and print it
    //                float pitch = liveLinkFace.getBlendshape(PyLiveLinkFace::FaceBlendShape::HeadPitch);
    //                qDebug() << liveLinkFace.name << pitch;
    //            }
    //        }
    //    }
    while (true){
        if(udpSocket.waitForReadyRead(-1)) {
            QByteArray datagram;
            datagram.resize(udpSocket.pendingDatagramSize());
            udpSocket.readDatagram(datagram.data(), datagram.size());
            bool success;
            PyLiveLinkFace liveLinkFace;
            success= liveLinkFace.decode(datagram);

            if (success) {
                // get the blendshape value for the HeadPitch and print it
                float pitch = liveLinkFace.getBlendshape(PyLiveLinkFace::FaceBlendShape::HeadPitch);
                qDebug() << liveLinkFace.name << pitch;
            }
            // 处理收到的数据
        }
    }
    return 0;
}
//#include <QtNetwork/QUdpSocket>
//#include <QByteArray>
//#include <QDataStream>
//#include <QDebug>

//int main(int argc, char *argv[]) {
//    QCoreApplication app(argc, argv);

//    // 创建UDP套接字
//    QUdpSocket socket;
//    if (!socket.bind(QHostAddress::Any, 3333)) {
//        qCritical() << "Bind failed:" << socket.errorString();
//        return 1;
//    }

//    // 接收数据
//    while (true) {
//        if (socket.hasPendingDatagrams()) {
//            QByteArray datagram;
//            datagram.resize(socket.pendingDatagramSize());
//            QHostAddress sender;
//            quint16 senderPort;
//            socket.readDatagram(datagram.data(), datagram.size(), &sender, &senderPort);
//            QDataStream stream(datagram);
//            stream.setByteOrder(QDataStream::BigEndian);
//            float data[3];
//            stream.readRawData((char*)&data, 12);
//            qDebug() << "Received data:" << qToBigEndian(data[0])<<","<< qToBigEndian(data[1])<<","<< qToBigEndian(data[2]) << "from" << sender.toString() << "port" << senderPort;
//        }
//        QCoreApplication::processEvents();
//    }

//    return 0;
//}
