# Cinder-Beat
a implementation of ofxBeat in Cinder.

## Description
This repository is the block that has ported [ofxBeat](https://github.com/darrenmothersele/ofxBeat) to Cinder.

## Install
Download or check out this repository into your Cinder/blocks directory, then use Tinderbox to create a new project using Beat.

## Usage
Include the header and create instance of BeatRef in your `.cpp` file: 
```c++
#include "Beat.hpp"
```


```c++
class BeatTemplateApp : public App {
  public:
	void setup() override;
	void mouseDown( MouseEvent event ) override;
	void update() override;
	void draw() override;
    
    BeatRef mBeat;
};
```

Create and setup at `setup()`:
```c++
void BeatTemplateApp::setup()
{
    mBeat = Beat::create();
    mBeat->setup();
}
```

Call update method at `update()`:
```c++
void BeatTemplateApp::update()
{
    mBeat->update(getElapsedSeconds() * 0.001f);
}
```

Then you're ready to get Kick, Snare or Hihat:
```c++
float kick = mBeat->getKick();
float snare = mBeat->getSnare();
float hihat = mBeat->getHihat();
```

You can also get Volume using `getVolume()`:
```c++
float volume = mBeat->getVolume();
```
