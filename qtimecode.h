#ifndef QTIMECODE_H
#define QTIMECODE_H
#include <QString>
#include <QStringList>
class QTimecode
{
public:
    static int fromString(QString timecodeString,int frameRate)
    {
        // extract hours, minutes, seconds, and frames from timecode string
        QStringList timecodeParts = timecodeString.split(":");
        int hours = timecodeParts[0].toInt();
        int minutes = timecodeParts[1].toInt();
        int seconds = timecodeParts[2].toInt();
        int frames = timecodeParts[3].toInt();

        // calculate total frames
        int totalFrames = (hours * frameRate * 3600) + (minutes * frameRate * 60) + (seconds * frameRate) + frames;
        return totalFrames;
    }
};

#endif // QTIMECODE_H
