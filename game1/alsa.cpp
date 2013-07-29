/* ALSA SYNTH DRIVER

  The synth is a tone generator. There are 3 oscillators and a filter.
  Programming is via 16 numbers. The arrangement is fixed.

  DCO1 -> DCO2 -> DCF0 -> OUT -> OK
   ^      DCO3 ----^       |
   |       ^               |
   +-------+---------------+

   Each DCO and DCF has an input which is a modulation. This is directed
   to the amplitude or Q, and the frequency. Both the amplitude/Q and freq
   are exponetial mapped, with the highest frequency being just under the
   filter stability point, and the lowest based on an octive divider toward
   zero frequency.

   Sequencing control is via indexes to groups of 16, or other commands, such
   as delay, or relative 16, for music perhaps. This will produce enough
   arcade sounds.
*/

#include "alsa.h"
#include <alsa/asoundlib.h>
#include <QtMath>

void Alsa::generate() {
    for(int i = 0; i < size; i++) {
        oscillate();
        hpf();
        filter();
        buffer[i] = out;
    }
    ready = true;//output it
}

void Alsa::oscillate() {
    for(int i = 0; i < NumDCO; i++) {
        int fmod = 0;
        int amod = 0;
        if(i) {
            fmod = amod = out; break;
        } else {
            fmod = amod = DCO[1][3]; break;
        }
        fmod = exp[((fmod * DCO[i][4]) >> expScale) + 2048];
        amod = exp[((amod * DCO[i][5]) >> expScale) + 2048];

        DCO[i][3] = ( ((exp[(DCO[i][2] >> 4) + 2048] * amod) >> divScale) *
                (DCO[i][0] -= exp[(DCO[i][1] >> 4) + 2048] + fmod))
                >> divScale;//pos move down saw
    }
    out = DCO[0][3];
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
    fmod = exp[((fmod * DCO[3][4]) >> expScale) + 2048];
    amod = exp[((amod * DCO[3][5]) >> expScale) + 2048];


    f = fmod + exp[(DCO[3][1] >> 4) + 2048];
    if(f < 0) f = 0;
    if(f > 65535) f = 65535;
    f = sin[f >> 4];
    q = amod + exp[(DCO[3][2] >> 4) + 2048];
    if(q < 0) q = 0;//it's inverse q actually!!
    if(q > 65536 * 3 / 2) q = 65536 * 3 / 2;
    q = (65536 * 3 / 2 - q) << 1;
    e2 = e2 + ((f * e1) >> divScale);
    h = out - e2 - ((q * e1) >> divScale);
    b = ((f * h) >> divScale) + e1;
    e1 = b;
    out = e2;
}

void Alsa::makePow() {
    //frequency scaling
    for(int i = 0; i < 4096; i++) {
        //a four octave range plus or minus
        exp[i] = (int16_t) (qPow(2, (i - (2048)) * 4 / 2048.0 ) * 100);
    }
    double f;
    for(int i = 0; i < 4096; i++) {
        f = (441.0 * exp[i]);
        //filter mapping of note to play
        sin[i] = (int16_t) (2 * qSin(3.141592658 * f/44100));
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

  /* Interleaved mode */
  snd_pcm_hw_params_set_access(handle, params,
                      SND_PCM_ACCESS_RW_INTERLEAVED);

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
  buffer = (int16_t *) malloc(size);
  for(int i = 0; i < size; i++) buffer[i] = 0;

  for(int i = 0; i < NumDCO; i++) {
      DCO[i][0] = 32768;//pos
      DCO[i][1] = 0;//freq
      DCO[i][2] = 0;//amp
      DCO[i][4] = 0;//freqm
      DCO[i][5] = 0;//ampm
  }

  /* We want to loop for 5 seconds */
  snd_pcm_hw_params_get_period_time(params,
                                    &val, &dir);

  out = 0;
  ready = true;
  timer = new QTimer(this);
  connect(timer, SIGNAL(timeout()), this, SLOT(loop()));
  timer->start(10);//fast timer
}

void Alsa::loop() {
    if(ready) {
        rc = snd_pcm_writei(handle, buffer, frames);
        if (rc == -EPIPE) { /* -EAGAIN processing needed for not ready !!!! */
          /* EPIPE means underrun */
          snd_pcm_prepare(handle);
        }
        if (rc != -EAGAIN) ready = false;
    }
    if(!ready) generate();
}

Alsa::~Alsa() {
  snd_pcm_drain(handle);
  snd_pcm_close(handle);
  free(buffer);
}



