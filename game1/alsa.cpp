/* ALSA SYNTH DRIVER

  The synth is a tone generator. There are 3 oscillators and a filter.
  Programming is via 16 numbers. The arrangement is fixed.

  DCO1 -> DCO2 -> DCF0 -> OUT -> OK
   ^      DCO3 ----^       |
   |       ^               |
   +-------+---------------+

   Each DCO and DCF has an input which is a modulation. This is directed
   to the amplitude or Q, and the frequency. Both the amplitude/Q and freq
   are !exponetial mapped, with the highest frequency being just over the
   filter stability point, and the lowest based on an octive divider toward
   zero frequency.

   Each oscillator/filter slot has 8 parameters controlled by indexing a
   timbre buffer. The timbre buffer is indexed to the next position on zero
   crossing to make a granular FM synth. The timbre buffer can be filled
   with sound, to make a random-ish mutate. The granularity is switched off
   when mutating the timbre. (maybe not needed later optimization).

   General note playing can set a tone Hz, and the volume also controls
   some of the tonal character too. LFO effects can be via long chains in
   the timbre. They are note dependant in rate though.

   This will produce enough arcade sounds.

   Later intended arrangements will have a few of the tracker usuals.
*/

#include "alsa.h"
#include <alsa/asoundlib.h>
#include <QtMath>

void Alsa::generate() {
    for(int i = 0; i < size; i++) {
        oscillate();
        hpf();
        filter();
        if(out > 32767) out = 32767;//clip
        if(out < -32767) out = -32767;
        buffer[render][i] = out;
    }
    if(render == 0) ready = true;//output it
}

int16_t *Alsa::setTimbre(int16_t *timbre) {
    int16_t *tmp = buffer[1];
    buffer[1] = timbre;
    return tmp;
}

void Alsa::switcher(int osc) {
    for(int i = 0; i < 4; i++) {
        //descreet granular FM
        DCO[osc][i] = *(masterP + i + DCO[osc][av]%(size / 4));
        DCO[osc][i + 4] = *(masterP + i + DCO[osc][fv]%(size / 4));;
    }
    normalize();
}

void Alsa::oscillate() {
    for(int i = 0; i < NumDCO; i++) {
        int fmod = 0;
        int amod = 0;
        if(i) {
            fmod = amod = out; break;
        } else {
            fmod = amod = DCO[1][o]; break;
        }
        fmod = (fmod * DCO[i][fm]) >> divScale;
        amod = (amod * DCO[i][am]) >> divScale;
        int16_t b4 = DCO[i][p];

        DCO[i][p] -= exp[((DCO[i][f] >> 4) & 2047) + 2048] / 3;
        if(b4 * DCO[i][p] < 0 && render == 0) switcher(i);

        DCO[i][o] = DCO[i][p] + fmod;//PM not FM, as easier
        //basically the index of modulation is easier
        //to control DC too
        //amplitude scaling
        DCO[i][o] = ( ((DCO[i][a] * amod) >> divScale) *
                DCO[i][o]) >> divScale;//pos move down saw
    }
    out = DCO[0][o];
}

void Alsa::hpf() {
    //filter out DC
    l = d2 + (d1 >> 12);
    out = out - l - d1;
    b = (out >> 12) + d1;
    d1 = b;
    d2 = l;
}

void Alsa::filter() {
    //state variable filter
    int fmod = 0;
    int amod = 0;
    fmod = amod = DCO[2][3];
    fmod = (fmod * DCO[3][fm]) >> divScale;
    amod = (amod * DCO[3][am]) >> divScale;

    ff = (DCO[3][f] + fmod) >> 4;
    if(ff < 0) ff = 0;
    if(ff > 4095) ff = 4095;
    ff = sin[ff];
    q = amod * DCO[3][a] >> divScale;
    //it's inverse q actually!!
    q = (q * q) >> divScale;//normalize positive
    q = 32767 / (q + 1);
    e2 = e2 + ((ff * e1) >> divScale);
    h = out - e2 - ((q * e1) >> divScale);
    b = ((ff * h) >> divScale) + e1;
    e1 = b;
    out = e2;
}

void Alsa::makePow() {
    //frequency scaling
    for(int i = 0; i < 4096; i++) {
        //a four octave range plus or minus (9 and a little octaves)
        //1800 picked for 60*60 harmonic matching!
        //tunes center 367.5Hz
        //the later 3 gets 4 times the tuning accuracy
        exp[i] = (int16_t) (qPow(2, (i - (2048)) * 3 / 1800.0 ) * 120 * 3);
    }
    double f;
    for(int i = 0; i < 4096; i++) {
        f = (367.5 * exp[i] / 120 * 3);
        //filter mapping of note to play
        //some of the high notes may have a negative number
        sin[i] = (int16_t) (qSin(3.141592658 * f/44100) * 65536);
        if(sin[i] < 0) sin[i] = 32767;//limit
    }
}

Alsa::Alsa() : QObject() {

    makePow();
  /* Open PCM device for playback. */
  rc = snd_pcm_open(&handle, "default",
    SND_PCM_STREAM_PLAYBACK, SND_PCM_NONBLOCK); /* !!!! NON-BLOCKING OPTION ???? */
  if (rc < 0) {
    rc = 0;
    return;
  }

  /* Allocate a hardware parameters object. */
  snd_pcm_hw_params_alloca(&params);

  /* Fill it in with default values. */
  snd_pcm_hw_params_any(handle, params);

  /* Interleaved mode NO ... MONO */
  //snd_pcm_hw_params_set_access(handle, params,
  //                    SND_PCM_ACCESS_RW_INTERLEAVED);

  /* Signed 16-bit little-endian format */
  snd_pcm_hw_params_set_format(handle, params,
                              SND_PCM_FORMAT_S16_LE);

  /* One channel (mono) */
  snd_pcm_hw_params_set_channels(handle, params, 1);

  /* 44100 bits/second sampling rate (CD quality) */
  val = 44100;
  snd_pcm_hw_params_set_rate_near(handle, params,
                                  &val, &dir);

  /* Set period size to 256 frames. */
  frames = 256;
  snd_pcm_hw_params_set_period_size_near(handle,
                              params, &frames, &dir);

  /* Write the parameters to the driver */
  rc = snd_pcm_hw_params(handle, params);
  if (rc < 0) {
    rc = 0;
    return;
  }

  /* Use a buffer large enough to hold one period */
  snd_pcm_hw_params_get_period_size(params, &frames,
                                    &dir);
  size = frames; /* 2 bytes/sample, 2 channels */
  buffer[0] = (int16_t *) malloc(size);
  buffer[1] = (int16_t *) malloc(size);
  for(int j = 0; j < 2; j++)
    for(int i = 0; i < size; i++) buffer[j][i] = 0;

  initialize();

  /* We want to loop for 5 seconds? */
  snd_pcm_hw_params_get_period_time(params,
                                    &val, &dir);

  out = 0;
  ready = true;
  mutate();
  timer = new QTimer(this);
  connect(timer, SIGNAL(timeout()), this, SLOT(loop()));
  timer->start(val / 2000);//fast timer
}

void Alsa::mutate() {
    render = 1;
    generate();
    render = 0;
}

void Alsa::play(int frequency, int volume, int pattern, int16_t *timbre) {
    masterF = frequency;
    masterA = volume;
    if(timbre) masterP = timbre;
    else masterP = buffer[1];//timbre buffer
    masterI = pattern%(size / 4);
    for(int i = 0; i < (NumDCO +1) * 8; i++) {
        *((int16_t *)DCO + i) = *(timbre + i + masterI);//initial note data
    }
    normalize();
}

void Alsa::normalize() {
    for(int i = 0; i < NumDCO + 1; i++) {
        DCO[i][f] += masterF;//freq
        DCO[i][a] += masterA;//amp
    }
}

void Alsa::initialize() {
    for(int i = 0; i < (NumDCO + 1) * 8; i++) {
        *(DCO + i) = 0;
        /* DCO[i][p] = 0;//pos
        DCO[i][f] = 0;//freq
        DCO[i][a] = 0;//amp
        DCO[i][fm] = 0;//freqm
        DCO[i][am] = 0;//ampm
        DCO[i][fv] = 0;
        DCO[i][av] = 0; */
    }
}

void Alsa::loop() {
    int rtmp = render;
    if(ready) {
        rc = snd_pcm_writei(handle, buffer[render = 0], frames);
        if (rc == -EPIPE) { /* -EAGAIN processing needed for not ready !!!! */
          /* EPIPE means underrun */
          snd_pcm_prepare(handle);
        }
        if (rc != -EAGAIN) ready = false;
    }
    if(!ready) generate();
    render = rtmp;
}

Alsa::~Alsa() {
  snd_pcm_drain(handle);
  snd_pcm_close(handle);
  free(buffer[1]);
  free(buffer[0]);
}



