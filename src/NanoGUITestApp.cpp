#include "cinder/app/App.h"
#include "cinder/app/RendererGl.h"
#include "cinder/gl/gl.h"

using namespace ci;
using namespace ci::app;
using namespace std;

class NanoGUITestApp : public App {
  public:
	void setup() override;
	void mouseDown( MouseEvent event ) override;
	void update() override;
	void draw() override;
};

void NanoGUITestApp::setup()
{
}

void NanoGUITestApp::mouseDown( MouseEvent event )
{
}

void NanoGUITestApp::update()
{
}

void NanoGUITestApp::draw()
{
	gl::clear( Color( 0, 0, 0 ) ); 
}

CINDER_APP( NanoGUITestApp, RendererGl )
