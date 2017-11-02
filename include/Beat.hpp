#pragma once

#include "cinder/Cinder.h"
#include "cinder/audio/InputNode.h"
#include "cinder/audio/MonitorNode.h"
#include "cinder/audio/Context.h"
#include "fft.hpp"

using namespace ci;
using namespace std;

typedef shared_ptr<class Beat> BeatRef;

class Beat{
public:
    enum BandType : int
    {
        KICK = 0,
        SNARE = 1,
        HIHAT = 2
    };
    
    static BeatRef create();
    Beat();
    
    void setup();
    void update(int);
    
    // Setter
    void isBeatDetect(bool b){ mBDetectBeat = b; }
    void setGain(float gain){ mGain = gain; }
    
    // Getter
    float getMagnitude(){ return *mMagnitude.get(); }
    int getBufferSize(){ return BUFFER_SIZE; }
    bool isBeat(const int subband);
    float getBand(const int i){ return mFftSubbands[i]; }
    bool isKick();
    float getKick(){ return mBeats[BandType::KICK]; }
    bool isSnare();
    float getSnare(){ return mBeats[BandType::SNARE]; }
    bool isHat();
    float getHihat(){ return mBeats[BandType::HIHAT]; }
    float getVolume();
    
private:
    static const int BUFFER_SIZE = 1024;
    static const int FFT_SIZE = 512;
    
    static const int FFT_BINS = 512;
    static const int FFT_SUBBANDS = 32;
    static const int ENERGY_HISTORY = 43;
    
    void calcFFT(const float*, const int, const int);
    void updateFFT();
    void updateBand(const bool, const int, const int);
    
    bool isBeatRange(const int low, const int high, const int threshold);
    
    unique_ptr<float[]> mFftSmoothed;
    unique_ptr<float[]> mAverageEnergy;
    unique_ptr<float[]> mFftVariance;
    unique_ptr<float[]> mBeatValueArray;
    unique_ptr<float[][ENERGY_HISTORY]> mEnergyHistory;
    unique_ptr<float[]> mInFft;
    
    int mHistoryPos;
    bool mFftInit;
    bool mBDetectBeat;
    float mGain;
    
    float mFftSubbands[FFT_SUBBANDS];
    int mBandTimes[3];
    int mBeatSizes[3];
    float mBeats[3];
    
    unique_ptr<float[]> mMagnitude;
    unique_ptr<float[]> mPhase;
    unique_ptr<float[]> mPower;
    unique_ptr<float[]> mAudioInput;
    unique_ptr<float[]> mMagnitudeAverage;
    unique_ptr<float[]> mMagnitudeAverageSnapshot;
    
    fft mMyfft;
    
    audio::InputDeviceNodeRef mInputDeviceNode;
    audio::MonitorNodeRef mMonitorNode;
};

