/*  The dynamic Alsa granular FM synth class.
    Developped from an ALSA demo application from a computer magazine.
    Adaptation (C)2013 Sleep Dream Games.

    */
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

public slots:
    void loop();

private:
    int out;
    void oscillate();
    void hpf();
    void filter();
    void normalize();
    int16_t exp[4096];//exponential curve
    int16_t sin[4096];//sine curve
    enum { NumDCO = 3 };
    int16_t DCO[NumDCO + 1][8];//and VCF fake
    enum {divScale = 15};//signed and 15
    enum {expScale = divScale + 4};//to signed and 11 bit
    int l, b, d2, d1;
    int e2, e1;
    int q, ff, h;
    void makePow();
    void generate();

    int rc;
    snd_pcm_t *handle;
    snd_pcm_hw_params_t *params;
    unsigned int val;
    int dir;
    snd_pcm_uframes_t frames;
    int16_t *buffer[2];
    QTimer * timer;
    bool ready;
    int render;

    //meta sequencer
    int masterF;
    int masterA;
    int16_t *masterP;
    int masterI;
    void switcher(int osc);

    enum {
        //amp
        o = 0,
        a = 1,
        am = 2,
        av = 3,
        //freq
        p = 4,
        f = 5,
        fm = 6,
        fv = 7
    };

public:
    int size;
    void initialize();
    void play(int frequency = 0, int volume = 0, int pattern = 0, int16_t *timbre = NULL);
    //evolver handling
    int16_t *setTimbre(int16_t *timbre);
    void mutate();
};

#endif // ALSA_H
