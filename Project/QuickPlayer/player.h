#ifndef PLAYER_H
#define PLAYER_H

#include <QObject>
extern "C"{
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>
#include <libswresample/swresample.h>
#include <libavutil/buffer.h>
}

#include <QAudioFormat>
#include <QAudioDeviceInfo>
#include <QAudioOutput>
#include <QImage>
#include <QDebug>
#include <QList>
#include <QMutex>

#define MyDebug() qDebug() << __FILE__ << __LINE__

class Player : public QObject
{
    Q_OBJECT
public:
    explicit Player(QObject *parent = nullptr);
    ~Player();

    bool open(QString filename);
    void pause();
    void resume();
    void triggler();
    void setSpeed(int speed);
    void seek(quint64 frame);

    quint64 getNBFrames()
    {
        return vStream->nb_frames;
    }

private:
    QString filename;
    AVFormatContext * ic;
    AVStream * aStream;
    AVStream * vStream;
    AVCodecContext * aCtx;
    AVCodecContext * vCtx;

    struct SwrContext * swr;
    uint8_t * out;
    int out_count;

    struct SwsContext * sws;
    QImage image;

    QAudioOutput * audioOutput;
    QIODevice * audioIO;
    QAudioFormat format;

    AVFrame * frm;

    AVSampleFormat getSampleFormat(QAudioFormat format);

    pthread_t tid_reader;
    pthread_t tid_decoder;

    //working the sub_thread
    static void * reader_func(void * ptr);
    void read();

    QList<AVPacket*> pkts;
    QMutex mutex;
    bool eof;
    bool exit;
    bool bPaused;
    bool seekFlag;
    int seekFrames;
    bool speedFlag;
    int speedRate;

    //working the sub_thread
    static void * decode_func(void * ptr);
    void decode();
    void decode_audio_packet(AVPacket * pkt);
    void decode_video_packet(AVPacket * pkt);
signals:
    void sigNewImage(QImage image);

public slots:
};

#endif // PLAYER_H
