#ifndef LIVELINKFACE_H
#define LIVELINKFACE_H
#include <QObject>
#include <QUdpSocket>
#include <QDataStream>
#include <QDebug>
#include <deque>
#include <chrono>
#include <QUuid>
#include <ctime>
#include <QtEndian>
#include "qtimecode.h"
/**
 * @brief The PyLiveLinkFace class 注意大小端是对单个数据来说的不是整个数据包,所以应该对单个数据处理大小端
 */
class PyLiveLinkFace : public QObject
{
    Q_OBJECT
public:
    enum FaceBlendShape {
        EyeBlinkLeft = 0,
        EyeLookDownLeft,
        EyeLookInLeft,
        EyeLookOutLeft,
        EyeLookUpLeft,
        EyeSquintLeft,
        EyeWideLeft,
        EyeBlinkRight,
        EyeLookDownRight,
        EyeLookInRight,
        EyeLookOutRight,
        EyeLookUpRight,
        EyeSquintRight,
        EyeWideRight,
        JawForward,
        JawLeft,
        JawRight,
        JawOpen,
        MouthClose,
        MouthFunnel,
        MouthPucker,
        MouthLeft,
        MouthRight,
        MouthSmileLeft,
        MouthSmileRight,
        MouthFrownLeft,
        MouthFrownRight,
        MouthDimpleLeft,
        MouthDimpleRight,
        MouthStretchLeft,
        MouthStretchRight,
        MouthRollLower,
        MouthRollUpper,
        MouthShrugLower,
        MouthShrugUpper,
        MouthPressLeft,
        MouthPressRight,
        MouthLowerDownLeft,
        MouthLowerDownRight,
        MouthUpperUpLeft,
        MouthUpperUpRight,
        BrowDownLeft,
        BrowDownRight,
        BrowInnerUp,
        BrowOuterUpLeft,
        BrowOuterUpRight,
        CheekPuff,
        CheekSquintLeft,
        CheekSquintRight,
        NoseSneerLeft,
        NoseSneerRight,
        TongueOut,
        HeadYaw,
        HeadPitch,
        HeadRoll,
        LeftEyeYaw,
        LeftEyePitch,
        LeftEyeRoll,
        RightEyeYaw,
        RightEyePitch,
        RightEyeRoll
    };

    PyLiveLinkFace(QString name="C++_LiveLinkFace",QString uuid=PyLiveLinkFace::generateUuid(),
                   int fps=60,int filter_size=5,QObject *parent = nullptr)
        : QObject(parent)
    {
        // 初始化一些参数
        this->uuid = uuid;
        this->name = name;
        this->fps = 60;
        this->_filter_size = filter_size;
        _version=6;
        std::chrono::system_clock::time_point now = std::chrono::system_clock::now();
        std::time_t now_c = std::chrono::system_clock::to_time_t(now);
        std::tm now_tm = *std::localtime(&now_c);
        QString timeStr = QString::asprintf("%02d:%02d:%02d:%03d",
                                            now_tm.tm_hour, now_tm.tm_min, now_tm.tm_sec,
                                            static_cast<int>(now.time_since_epoch().count() % 1000));
        _frames = QTimecode::fromString(timeStr, fps);
        _sub_frame = 1056060032; // 我不知道如何计算这个
        _denominator = fps / 60; // 大多数情况下是1
        _blend_shapes.assign(61,0.0);
        for(int i=0; i<61; i++) {
            std::deque<float> dq(_filter_size,0.0f);
            _old_blend_shapes.push_back(dq);
        }
    }
    void set_blendshape(FaceBlendShape index,float value,bool no_filter=true ){
        if (no_filter){
            _blend_shapes[index]=value;
        }else{
            if(_old_blend_shapes[index].size()==_filter_size){
                _old_blend_shapes[index].pop_front();
            }
            _old_blend_shapes[index].push_back(value);
            float filterd_value =0;
            for(float v:_old_blend_shapes[index]){
                filterd_value+=v;
            }
            if(_old_blend_shapes[index].size()>0){
                filterd_value/=_old_blend_shapes[index].size();
            }
            _blend_shapes[index] = filterd_value;
        }

    }
    float getBlendshape(FaceBlendShape index){
        return _blend_shapes[index];
    }
    QByteArray encode() const {
        QByteArray uuid_bytes = uuid.toUtf8();
        QByteArray name_bytes = name.toUtf8();

        QByteArray blend_shapes_bytes;
        QDataStream blend_shapes_stream(&blend_shapes_bytes, QIODevice::WriteOnly);
        blend_shapes_stream.setByteOrder(QDataStream::BigEndian);
        for (auto bs : _blend_shapes) {
            blend_shapes_stream << bs;
        }

        QByteArray packet_bytes;
        QDataStream packet_stream(&packet_bytes, QIODevice::WriteOnly);
        packet_stream.setByteOrder(QDataStream::LittleEndian);
        packet_stream<< quint32(6);
        packet_stream.setByteOrder(QDataStream::BigEndian);
        packet_stream << uuid_bytes << qint32(name_bytes.size()) << name_bytes <<
                         qint32(_frames) << qint32(_sub_frame) << qint32(fps) << qint32(_denominator) <<
                         quint8(61) << blend_shapes_bytes;
        return packet_bytes;
    }
    bool decode(QByteArray bytesData) {
        int version;
        QString uuid;
        QString name;
        int nameLength;
        int frameNumber;
        int subFrame;
        int fps;
        int denominator;
        quint8 dataLength;
        QByteArray blendShapes;
        QDataStream stream(bytesData);
        stream.setByteOrder(QDataStream::LittleEndian);

        stream >> version;

        if (version < 6) {
            return false;
        }
        stream.setByteOrder(QDataStream::BigEndian);
        uuid = QString(stream.device()->read(37));
        if (stream.status() != QDataStream::Ok) {
            return false;
        }
        //qDebug() << uuid;
        stream >> nameLength;

        name = QString(stream.device()->read(nameLength));
        if (stream.status() != QDataStream::Ok) {
            return false;
        }
        //qDebug() << name;

        stream >> frameNumber>>subFrame>> fps >> denominator >> dataLength;

        if (dataLength != 61) {
            qWarning() << "Data does not contain a valid PyLiveLinkFace packet.";
            return false;
        }
        blendShapes.resize(61 * sizeof(float));
        stream.readRawData(blendShapes.data(), blendShapes.size());
        this->_version = version;
        this->uuid = uuid;
        this->name = name;
        this->_frames = frameNumber;
        this->_sub_frame = subFrame;
        this->fps = fps;
        this->_denominator = denominator;


        const float* blendShapesData = reinterpret_cast<const float*>(blendShapes.constData());
        for(int i=0;i<61;++i){
            this->_blend_shapes[i]=qToBigEndian(blendShapesData[i]);
        }
        //std::for_each(this->_blend_shapes.begin(), this->_blend_shapes.end(), [&](float v) {qDebug() << v; });
        return true;
    }

    /**
     * @brief generateUuid  函数返回一个 36 字符的字符串，
     * 形如 "e6b3a1f1-09a4-4d39-9b4a-14d675c0d8bc"，
     * 其中包含了32个16进制的字符和4个短横线
     * @return
     */
    static QString generateUuid() {
        QUuid uuid = QUuid::createUuid();
        return uuid.toString();
    }
public:
    int fps;
    QString uuid;
    QString name;
    int _filter_size;
    int _version;
    int _frames;
    int _sub_frame;
    int _denominator;
    std::vector<float> _blend_shapes;
    std::vector<std::deque<float>> _old_blend_shapes;

};

#endif // LIVELINKFACE_H
