
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
	SpoutOut					mSpoutOut;
};



BatchassKinectApp::BatchassKinectApp()
	: mSpoutOut("Kinec", app::getWindowSize())
{
}
void BatchassKinectApp::draw()
{
	gl::viewport(getWindowSize());
	gl::clear();
	gl::setMatricesWindow(getWindowSize());
	gl::enableAlphaBlending();

	if (mSurfaceColor) {
		gl::TextureRef tex = gl::Texture::create(*mSurfaceColor);
		gl::draw(tex, tex->getBounds(), Rectf(vec2(0.0f), getWindowCenter()));
	}
	if (mChannelDepth) {
		gl::TextureRef tex = gl::Texture::create(*Kinect2::channel16To8(mChannelDepth));
		gl::draw(tex, tex->getBounds(), Rectf(getWindowCenter().x, 0.0f, (float)getWindowWidth(), getWindowCenter().y));
	}
	if (mChannelInfrared) {
		gl::TextureRef tex = gl::Texture::create(*mChannelInfrared);
		gl::draw(tex, tex->getBounds(), Rectf(0.0f, getWindowCenter().y, getWindowCenter().x, (float)getWindowHeight()));
	}
	if (mChannelBodyIndex) {
		gl::TextureRef tex = gl::Texture::create(*Kinect2::colorizeBodyIndex(mChannelBodyIndex));
		gl::draw(tex, tex->getBounds(), Rectf(getWindowCenter(), vec2(getWindowSize())));
	}
	mSpoutOut.sendViewport();
	mParams->draw();
}

void BatchassKinectApp::setup()
{
	gl::enable(GL_TEXTURE_2D);

	mFrameRate = 0.0f;
	mFullScreen = false;

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
	mParams->addParam("Frame rate", &mFrameRate, "", true);
	mParams->addParam("Full screen", &mFullScreen).key("f");
	mParams->addButton("Quit", [&]() { quit(); }, "key=q");
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
	settings->prepareWindow(Window::Format().size(800, 600).title("Basic App"));
	settings->setFrameRate(60.0f);
})

//CINDER_APP( BatchassKinectApp, ci::app::RendererGl )
	
