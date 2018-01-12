#include "player.h"
#include <QThread>

void *Player::reader_func(void *ptr)
{
    Player * player = (Player *)ptr;
    player->read();
    return NULL;
}



void *Player::decode_func(void *ptr)
{
    Player * player = (Player *)ptr;
    player->decode();
    return NULL;
}
Player::Player(QObject *parent) : QObject(parent)
{
    ic = NULL;
    aCtx = NULL;
    vCtx = NULL;
    swr = NULL;
    sws = NULL;
    out = NULL;
    audioOutput = NULL;
    audioIO = NULL;
    frm = NULL;

    tid_decoder = -1;
    tid_reader = -1;

    exit = false;
}

Player::~Player()
{
    exit = true;

    if((int)tid_decoder != -1) pthread_join(tid_decoder,NULL);

    if((int)tid_reader != -1) pthread_join(tid_reader,NULL);

    for(int i = 0;i< pkts.size();++i)
    {
        AVPacket * pkt = pkts.at(i);
        av_packet_free(&pkt);
    }

    if(frm) av_frame_free(&frm);

    if(audioOutput)
    {
        audioOutput->stop();
        delete audioOutput;
    }

    if(out) delete[] out;
    if(sws) sws_freeContext(sws);
    if(swr) swr_free(&swr);
    if(vCtx) avcodec_free_context(&vCtx);
    if(aCtx) avcodec_free_context(&aCtx);
    if(ic) avformat_free_context(ic);
}

bool Player::open(QString filename)
{
    bool result = false;

    do{

        this->filename = filename;
        int ret = avformat_open_input(&ic,filename.toLocal8Bit().data(),NULL,NULL);
        if(ret != 0)
        {
            MyDebug() << "open error";
            break;
        }

        //to find the info in stream
        ret = avformat_find_stream_info(ic,NULL);

        if(ret != 0)
        {
            MyDebug() << "find stream error";
            break;
        }

        //to get the streams in audio and video
        for(int i = 0; i< (int)ic->nb_streams;i++)
        {
            if(ic->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_AUDIO)
            {
                aStream = ic->streams[i];
            }
            if(ic->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO)
            {
                vStream = ic->streams[i];
            }
        }

        //get the decoder of video and audio
        AVCodec * codec;

        codec = avcodec_find_decoder(vStream->codecpar->codec_id);
        vCtx = avcodec_alloc_context3(codec);
        avcodec_parameters_to_context(vCtx,vStream->codecpar);
        avcodec_open2(vCtx,codec,NULL);

        codec = avcodec_find_decoder(aStream->codecpar->codec_id);
        aCtx = avcodec_alloc_context3(codec);
        avcodec_parameters_to_context(aCtx,aStream->codecpar);
        avcodec_open2(aCtx,codec,NULL);

        //to get the quantities of frames in video,for PAUSE and SEEK
        MyDebug() << "frames:" <<vStream->nb_frames << vStream->duration;
        MyDebug() << "time_base:"<<vStream->time_base.num << vStream->time_base.den;

        //initialize the device which playing audio
        format.setByteOrder(QAudioFormat::BigEndian);
        format.setChannelCount(2);
        format.setSampleRate(44100);
        format.setSampleSize(8);
        format.setSampleType(QAudioFormat::Float);

        QAudioDeviceInfo deviceInfo =
                QAudioDeviceInfo::defaultOutputDevice();

        if(! deviceInfo.isFormatSupported(format))
        {
            format = deviceInfo.nearestFormat(format);
        }

        out_count = 4410;
        out = new uint8_t[out_count * format.channelCount()*format.sampleSize()/8];

        //initialize swr
        swr = swr_alloc_set_opts(NULL,
                           av_get_default_channel_layout(format.channelCount()),
                           getSampleFormat(format),
                           format.sampleRate(),
                           aCtx->channel_layout,
                           aCtx->sample_fmt,
                           aCtx->sample_rate,0,NULL);
        swr_init(swr);

        audioOutput = new QAudioOutput(format);
        audioIO = audioOutput->start();

        //intialize sws
        sws = sws_getCachedContext(NULL,
                                   vCtx->width,
                                   vCtx->height,
                                   vCtx->pix_fmt,
                                   vCtx->width,
                                   vCtx->height,
                                   AV_PIX_FMT_RGBA,
                                   SWS_BICUBIC,NULL,NULL,NULL);
        image = QImage(vCtx->width,vCtx->height,QImage::Format_RGBA8888);
        frm = av_frame_alloc();
        eof = false;
        seekFlag = false;
        speedFlag = false;

        //start the thread
        pthread_create(&tid_reader,NULL,reader_func,this);
        pthread_create(&tid_decoder,NULL,decode_func,this);

        result = true;
    }
    while(0);
    MyDebug() << "opensuccess";
    return result;
}


void Player::pause()
{
    bPaused = true;
}

void Player::resume()
{
    bPaused = false;
}


void Player::triggler()
{
    bPaused = !bPaused;
}

void Player::setSpeed(int speed)
{
    speedRate = speed;
    speedFlag = true;
}

void Player::seek(quint64 frame)
{
    seekFrames = frame;
    seekFlag = true;
}

AVSampleFormat Player::getSampleFormat(QAudioFormat format)
{
    int sampleSize = format.sampleSize()/8;
    QAudioFormat::SampleType sampleType = format.sampleType();

    if(sampleSize == 1)
    {
        return AV_SAMPLE_FMT_U8;
    }
    if(sampleSize == 2)
    {
        return AV_SAMPLE_FMT_S16;
    }
    if(sampleSize == 4)
    {
        if(sampleType == QAudioFormat::Float)
            return AV_SAMPLE_FMT_FLT;
        else
            return AV_SAMPLE_FMT_S32;

    }
    MyDebug() << "getSampleFormat return NONE";
    return AV_SAMPLE_FMT_NONE;

}

void Player::read()
{
    while(!exit)
    {
        if(seekFlag)
        {
            av_seek_frame(ic,vStream->index,seekFrames*1.0/vStream->nb_frames*vStream->duration,0);
            seekFlag = false;
            //to lock and clear the buffer
            mutex.lock();
            for(int i = 0;i < pkts.size();++i)
            {
                AVPacket * pkt = pkts.at(i);
                av_packet_free(&pkt);
            }

            pkts.clear();
            mutex.unlock();
        }

        //limit the size of the area of buffer
        mutex.lock();
        if(pkts.size() >= 100)
        {
            mutex.unlock();

            QThread::msleep(100);
            continue;
        }
        mutex.unlock();

        AVPacket * pkt = av_packet_alloc();
        if(av_read_frame(ic,pkt) != 0)
        {
            break;
        }
        mutex.lock();
        pkts.push_back(pkt);
        mutex.unlock();
    }

    //the end of file
    eof = true;
}

void Player::decode()
{
    while(1)
    {
        if(bPaused)
        {
            QThread::sleep(100);
            continue;
        }

        AVPacket * pkt = NULL;
        mutex.lock();

        if(pkts.size() > 0)
        {
            pkt = pkts.first();
            pkts.removeFirst();
        }
        mutex.unlock();

        if(pkt == NULL)
        {
            if(eof)
            {
                MyDebug() << 1;
                break;
            }
            //to check the file is end or not.
            QThread::msleep(100);
            continue;
        }

        if(pkt->stream_index == aStream->index)
        {
            decode_audio_packet(pkt);
        }
        else if(pkt->stream_index == vStream->index)
        {
            decode_video_packet(pkt);
        }
        av_packet_free(&pkt);
    }
}

void Player::decode_audio_packet(AVPacket *pkt)
{
    if(avcodec_send_packet(aCtx,pkt) != 0)
        return;
    if(avcodec_receive_frame(aCtx,frm) != 0)
        return;
    int out_samples = swr_convert(swr,&out,out_count,(const uint8_t **)frm->data,frm->nb_samples);
    int out_bytes = out_samples * format.channelCount()*format.sampleSize()/8;
    int w_bytes = 0;

    while(w_bytes != out_bytes)
    {
        int ret = audioIO->write((char *)out +w_bytes,out_bytes - w_bytes);
        if(ret > 0)
        {
            w_bytes += ret;
        }
        else if(ret == 0)
        {
            audioIO->waitForBytesWritten(1000);
        }
        else
        {
            break;
        }
    }
}

void Player::decode_video_packet(AVPacket *pkt)
{
    if(avcodec_send_packet(vCtx,pkt) != 0)
    {
        return;
    }
    if(avcodec_receive_frame(vCtx,frm) != 0)
    {
        return;
    }

    if(speedFlag)
    {
        swr_free(&swr);

        swr = swr_alloc_set_opts(NULL,
                                 av_get_default_channel_layout(format.channelCount()),
                                 getSampleFormat(format),
                                 format.sampleRate()/(speedRate*1.0/100),
                                 aCtx->channel_layout,
                                 aCtx->sample_fmt,
                                 aCtx->sample_rate,
                                 0,NULL);

        swr_init(swr);
    }

    //sws
    uint8_t * const dst[] = {image.bits()};
   // MyDebug() << image.bits();
    const int dstStride[] = {image.width()*4};
    // MyDebug() << image.width() * 4;
    sws_scale(sws,frm->data,frm->linesize,0,frm->height,dst,dstStride);
    emit this->sigNewImage(image);
}

