#include "Beat.hpp"

BeatRef Beat::create()
{
    return BeatRef(new Beat());
}

Beat::Beat():mHistoryPos(0), mFftInit(true), mBDetectBeat(true), mGain(1.0f)
{
    mBandTimes[BandType::KICK] = 0;
    mBandTimes[BandType::SNARE] = 0;
    mBandTimes[BandType::HIHAT] = 0;
    mBeatSizes[BandType::KICK] = 0;
    mBeatSizes[BandType::SNARE] = 0;
    mBeatSizes[BandType::HIHAT] = 0;
    
    mFftSmoothed = make_unique<float[]>(FFT_BINS);
    mAverageEnergy = make_unique<float[]>(FFT_SUBBANDS);
    mFftVariance = make_unique<float[]>(FFT_SUBBANDS);
    mBeatValueArray = make_unique<float[]>(FFT_SUBBANDS);
    mEnergyHistory = make_unique<float[][ENERGY_HISTORY]>(FFT_SUBBANDS);
    
    for(int i=0; i<FFT_SIZE; i++){
        mFftSmoothed[i] = 0;
    }
    
    for(int i=0; i<FFT_SUBBANDS; i++){
        for(int j=0; j<ENERGY_HISTORY; j++){
            mEnergyHistory[i][j] = 0;
        }
        mFftSubbands[i] = 0;
        mAverageEnergy[i] = 0;
        mFftVariance[i] = 0;
        mBeatValueArray[i] = 0;
    }
    
    mMagnitude = make_unique<float[]>(FFT_SIZE);
    mPhase = make_unique<float[]>(FFT_SIZE);
    mPower = make_unique<float[]>(FFT_SIZE);
    mAudioInput = make_unique<float[]>(BUFFER_SIZE);
    mMagnitudeAverage = make_unique<float[]>(FFT_SIZE);
    mMagnitudeAverageSnapshot = make_unique<float[]>(FFT_SIZE);
    
    for(int i=0; i<FFT_SIZE; i++){
        mMagnitude[i] = 0;
        mPhase[i] = 0;
        mPower[i] = 0;
        mMagnitudeAverage[i] = 0;
        mMagnitudeAverageSnapshot[i] = 0;
    }
}

void Beat::setup()
{
    auto ctx = audio::Context::master();
    
    mInputDeviceNode = ctx->createInputDeviceNode();
    auto format = audio::MonitorNode::Format().windowSize(FFT_SIZE);
    mMonitorNode = ctx->makeNode(new audio::MonitorNode(format));
    
    mInputDeviceNode >> mMonitorNode;
    mInputDeviceNode->enable();
    ctx->enable();
}

void Beat::calcFFT(const float* input, const int bufferSize, const int nChannels)
{
    memcpy(mAudioInput.get(), input, sizeof(float) * bufferSize);
    float avgPower = 0.0f;
    mMyfft.powerSpectrum(0, FFT_SIZE, mAudioInput.get(), BUFFER_SIZE, mMagnitude.get(), mPhase.get(), mPower.get(), &avgPower);
    
    for(int i=0; i<FFT_SIZE; i++){
        mMagnitude[i] = powf(mMagnitude[i], 0.5);
    }
    
    for(int i=0; i<FFT_SIZE; i++){
        float x = 0.085;
        mMagnitudeAverage[i] = (mMagnitude[i] * x) + (mMagnitudeAverage[i] * (1 - x));
    }
}

void Beat::updateFFT()
{
    if(mFftInit){
        mInFft = move(mMagnitude);
        for(int i=0; i<FFT_SIZE; i++){
            if(mFftSmoothed[i] < mInFft[i]){
                mFftSmoothed[i] = mInFft[i];
            }
            mFftSmoothed[i] *= 0.90f;
        }
        
        if(mBDetectBeat){
            for(int i=0; i<FFT_SUBBANDS; i++){
                mFftSubbands[i] = 0;
                
                for(int b=0; b<FFT_SIZE/FFT_SUBBANDS; b++){
                    mFftSubbands[i] += mInFft[i*(FFT_SIZE/FFT_SUBBANDS)+b];
                }
                mFftSubbands[i] = mFftSubbands[i] * static_cast<float>(FFT_SUBBANDS) / static_cast<float>(FFT_SIZE);
                
                for(int b=0; b<FFT_SIZE/FFT_SUBBANDS; b++){
                    mFftVariance[i] += pow(mInFft[i*(FFT_SIZE/FFT_SUBBANDS)+b] - mFftSubbands[i], 2);
                }
                mFftVariance[i] = mFftVariance[i] * static_cast<float>(FFT_SUBBANDS) / static_cast<float>(FFT_SIZE);
                
                mBeatValueArray[i] = (-0.0025714 * mFftVariance[i]) + 1.35;
            }
            
            for(int i=0; i<FFT_SUBBANDS; i++){
                mAverageEnergy[i] = 0;
                for(int h=0; h<ENERGY_HISTORY; h++){
                    mAverageEnergy[i] += mEnergyHistory[i][h];
                }
                mAverageEnergy[i] /= ENERGY_HISTORY;
            }
            
            for(int i=0; i<FFT_SUBBANDS; i++){
                mEnergyHistory[i][mHistoryPos] = mFftSubbands[i];
            }
            
            mHistoryPos = (mHistoryPos + 1) % ENERGY_HISTORY;
        }
    }
    mMagnitude = move(mInFft);
}

void Beat::updateBand(const bool a, const int b, const int t)
{
    if(a){
        mBeats[b] = 1;
        mBeatSizes[b] = t - mBandTimes[b];
        mBandTimes[b] = t;
    }
    else{
        int span = t - mBandTimes[b];
        if(span < mBeatSizes[b] && mBeatSizes[b] > 0){
            mBeats[b] = ci::lmap(span, 0, mBeatSizes[b], 1, 0);
        }
        else{
            mBeats[b] = 0;
        }
    }
}

void Beat::update(int t)
{
    if(ci::app::getElapsedSeconds() < 0.5) return;
    
    const audio::Buffer& buffer = mMonitorNode->getBuffer();
    calcFFT(mMonitorNode->getBuffer().getData(), buffer.getSize(), buffer.getNumChannels());
    
    updateFFT();
    updateBand(isKick(), BandType::KICK, t);
    updateBand(isSnare(), BandType::SNARE, t);
    updateBand(isHat(), BandType::HIHAT, t);
}

bool Beat::isBeatRange(const int low, const int high, const int threshold)
{
    int num = 0;
    for(int i=low; i<high+1; i++){
        if(isBeat(i)) num++;
    }
    return num > threshold;
}

bool Beat::isBeat(const int subband)
{
    return mFftSubbands[subband] > mAverageEnergy[subband] * mBeatValueArray[subband];
}

bool Beat::isKick()
{
    return isBeat(BandType::KICK);
}

bool Beat::isSnare()
{
    int low = 1;
    int hi = FFT_SUBBANDS / 3;
    int thresh = (hi-low) / 3;
    return isBeatRange(low, hi, thresh);
}

bool Beat::isHat()
{
    int low = FFT_SUBBANDS / 2;
    int hi = FFT_SUBBANDS - 1;
    int thresh = (hi-low) / 3;
    return isBeatRange(low, hi, thresh);
}

float Beat::getVolume()
{
    float volume = 0.0f;
    for(int i=0; i<32; i++){
        float band = getBand(i);
        volume += band;
    }
    volume *= 0.03125f * mGain;
    return volume;
}

