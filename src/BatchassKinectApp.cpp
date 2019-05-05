
#include "cinder/app/App.h"
#include "cinder/app/RendererGl.h"
#include "cinder/gl/gl.h"
#include "cinder/gl/Pbo.h"
#include "cinder/gl/Texture.h"
#include "cinder/params/Params.h"

#include "Kinect2.h"
// Spout Output
#include "CiSpoutOut.h"

using namespace ci;
using namespace ci::app;
using namespace std;
class BatchassKinectApp : public ci::app::App
{
public:
	BatchassKinectApp();
	void						draw() override;
	void						setup() override;
	void						update() override;
private:
	Kinect2::DeviceRef			mDevice;
	ci::Channel8uRef			mChannelBodyIndex;
	ci::Channel16uRef			mChannelDepth;
	ci::Channel16uRef			mChannelInfrared;
	ci::Surface8uRef			mSurfaceColor;

	float						mFrameRate;
	bool						mFullScreen;
	ci::params::InterfaceGlRef	mParams;
	int							mSend;
	SpoutOut					mSpoutOut;
};



BatchassKinectApp::BatchassKinectApp()
	: mSpoutOut("Kinec", app::getWindowSize())
{
}

void BatchassKinectApp::setup()
{
	gl::enable(GL_TEXTURE_2D);

	mFrameRate = 0.0f;
	mFullScreen = false;
	mSend = 2;
	mDevice = Kinect2::Device::create();
	mDevice->start();
	mDevice->connectBodyIndexEventHandler([&](const Kinect2::BodyIndexFrame& frame)
	{
		mChannelBodyIndex = frame.getChannel();
	});
	mDevice->connectColorEventHandler([&](const Kinect2::ColorFrame& frame)
	{
		mSurfaceColor = frame.getSurface();
	});
	mDevice->connectDepthEventHandler([&](const Kinect2::DepthFrame& frame)
	{
		mChannelDepth = frame.getChannel();
	});
	mDevice->connectInfraredEventHandler([&](const Kinect2::InfraredFrame& frame)
	{
		mChannelInfrared = frame.getChannel();
	});

	mParams = params::InterfaceGl::create("Params", ivec2(200, 100));
	mParams->addParam("spOut", &mSend).key("o");
	mParams->addParam("Frame rate", &mFrameRate, "", true);
	mParams->addParam("Full screen", &mFullScreen).key("f");
	mParams->addButton("Quit", [&]() { quit(); }, "key=q");
}
void BatchassKinectApp::draw()
{
	gl::viewport(getWindowSize());
	gl::clear();
	gl::setMatricesWindow(getWindowSize());
	gl::enableAlphaBlending();
	switch (mSend)
	{
	case 0:
	if (mSurfaceColor) {
		gl::TextureRef tex = gl::Texture::create(*mSurfaceColor);
		gl::draw(tex, tex->getBounds(), getWindowBounds());
	}
		break;
	case 2:
	if (mChannelInfrared) {
		gl::TextureRef tex = gl::Texture::create(*mChannelInfrared);
		gl::draw(tex, tex->getBounds(), getWindowBounds());
	}
		break;
	case 3:
	if (mChannelBodyIndex) {
		gl::TextureRef tex = gl::Texture::create(*Kinect2::colorizeBodyIndex(mChannelBodyIndex));
		gl::draw(tex, tex->getBounds(), getWindowBounds());
	}
		break;
	default:
		// 1 or others
	if (mChannelDepth) {
		gl::TextureRef tex = gl::Texture::create(*Kinect2::channel16To8(mChannelDepth));
		gl::draw(tex, tex->getBounds(), getWindowBounds());
	}		
	break;
	}
	mSpoutOut.sendViewport();	

	if (mSurfaceColor) {
		gl::TextureRef tex = gl::Texture::create(*mSurfaceColor);
		//gl::draw(tex, tex->getBounds(), Rectf(vec2(0.0f), getWindowCenter()));
		gl::draw(tex, tex->getBounds(), Rectf(0.0f, 0.0f, 200.0f, 100.0f));
	}
	if (mChannelDepth) {
		gl::TextureRef tex = gl::Texture::create(*Kinect2::channel16To8(mChannelDepth));
		gl::draw(tex, tex->getBounds(), Rectf(200.0f, 0.0f, 400.0f, 100.0f));
	}
	if (mChannelInfrared) {
		gl::TextureRef tex = gl::Texture::create(*mChannelInfrared);
		gl::draw(tex, tex->getBounds(), Rectf(400.0f, 0.0f, 600.0f, 100.0f));
	}
	if (mChannelBodyIndex) {
		gl::TextureRef tex = gl::Texture::create(*Kinect2::colorizeBodyIndex(mChannelBodyIndex));
		gl::draw(tex, tex->getBounds(), Rectf(600.0f, 0.0f, 800.0f, 100.0f));
	}
	

	mParams->draw();
}

void BatchassKinectApp::update()
{
	mFrameRate = getAverageFps();

	if (mFullScreen != isFullScreen()) {
		setFullScreen(mFullScreen);
		mFullScreen = isFullScreen();
	}
}

CINDER_APP(BatchassKinectApp, RendererGl, [](App::Settings* settings)
{
	settings->prepareWindow(Window::Format().size(800, 600).title("Kinect Basic"));
	settings->setFrameRate(60.0f);
})

//CINDER_APP( BatchassKinectApp, ci::app::RendererGl )
	
