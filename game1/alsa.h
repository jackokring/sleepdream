#ifndef ALSA_H
#define ALSA_H

/* Use the newer ALSA API */
#define ALSA_PCM_NEW_HW_PARAMS_API

#include <alsa/asoundlib.h>
#include <QtGui>

class Alsa : public QObject
{
    Q_OBJECT

public:
    Alsa();
    ~Alsa();

    int rc;
    int size;
    snd_pcm_t *handle;
    snd_pcm_hw_params_t *params;
    unsigned int val;
    int dir;
    snd_pcm_uframes_t frames;
    int16_t *buffer;
    QTimer * timer;
    bool ready;

public slots:
    void loop();

private:
    int out;
    void oscillate();
    void hpf();
    void filter();
    int16_t exp[4096];//exponential curve
    int16_t sin[4096];//sine curve
    enum { NumDCO = 3 };
    int DCO[NumDCO + 1][6];//and VCF fake
    enum {divScale = 16};
    enum {expScale = 16 + 4};
    int l, b, d2, d1;
    int e2, e1;
    int q, f, h;
    void makePow();
    void generate();

};

#endif // ALSA_H
