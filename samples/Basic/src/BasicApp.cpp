#include "cinder/app/App.h"
#include "cinder/app/RendererGl.h"
#include "cinder/gl/gl.h"

#include "Beat.hpp"

using namespace ci;
using namespace ci::app;
using namespace std;

class BasicApp : public App {
  public:
	void setup() override;
	void update() override;
	void draw() override;
    
    BeatRef mBeat;
};

void BasicApp::setup()
{
    mBeat = Beat::create();
    mBeat->setup();
}

void BasicApp::update()
{
    mBeat->update(getElapsedSeconds() * 0.001f);
    getWindow()->setTitle(to_string(getAverageFps()));
}

void BasicApp::draw()
{
    gl::clear();
    gl::setMatricesWindow(getWindowSize());
    
    gl::drawSolidCircle(vec2(getWindowWidth() * 0.5 - 200, getWindowHeight() * 0.5), 50 * mBeat->getKick());
    gl::drawSolidCircle(vec2(getWindowWidth() * 0.5, getWindowHeight() * 0.5), 50 * mBeat->getSnare());
    gl::drawSolidCircle(vec2(getWindowWidth() * 0.5 + 200, getWindowHeight() * 0.5), 50 * mBeat->getHihat());
    
    cout << "kick : " << mBeat->isKick() << endl;
    cout << "snare : " << mBeat->isSnare() << endl;
    cout << "hihat : " << mBeat->isHat() << endl;
    cout << "volume : " << mBeat->getVolume() << endl;
}

CINDER_APP( BasicApp, RendererGl )
