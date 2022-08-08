/*
 Copyright (c) 2013-2022, Bruce Lane - All rights reserved.
 This code is intended for use with the Cinder C++ library: http://libcinder.org

 Using Cinder-Warping from Paul Houx.

 Cinder-Warping is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.

 Cinder-Warping is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with Cinder-Warping.  If not, see <http://www.gnu.org/licenses/>.
 */
 
#include "cinder/app/App.h"
#include "cinder/app/RendererGl.h"
#include "cinder/gl/gl.h"

// Animation
#include "VDAnimation.h"
// Session Facade
#include "VDSessionFacade.h"
// Spout
#include "CiSpoutOut.h"
// Uniforms
#include "VDUniforms.h"
// Params
#include "VDParams.h"
// Mix
#include "VDMix.h"
#include "cinder/gl/Pbo.h"
#include "cinder/gl/Texture.h"
#include "cinder/params/Params.h"

#include "Kinect2.h"
// UI
#define IMGUI_DISABLE_OBSOLETE_FUNCTIONS 1
#include "VDUI.h"
#define IM_ARRAYSIZE(_ARR)			((int)(sizeof(_ARR)/sizeof(*_ARR)))
using namespace ci;
using namespace ci::app;
using namespace videodromm;

class BatchassKinectApp : public App {
public:
	BatchassKinectApp();
	void cleanup() override;
	void update() override;
	void draw() override;
	void resize() override;
	void mouseMove(MouseEvent event) override;
	void mouseDown(MouseEvent event) override;
	void mouseDrag(MouseEvent event) override;
	void mouseUp(MouseEvent event) override;
	void keyDown(KeyEvent event) override;
	void keyUp(KeyEvent event) override;
	void fileDrop(FileDropEvent event) override;
private:
	// Settings
	VDSettingsRef					mVDSettings;
	// Animation
	VDAnimationRef					mVDAnimation;
	// Session
	VDSessionFacadeRef				mVDSessionFacade;
	// Mix
	VDMixRef						mVDMix;
	// Uniforms
	VDUniformsRef					mVDUniforms;
	// Params
	VDParamsRef						mVDParams;
	// UI
	VDUIRef							mVDUI;

	bool							mFadeInDelay = true;
	void							toggleCursorVisibility(bool visible);
	SpoutOut 						mSpoutOut;
	Kinect2::DeviceRef			mDevice;
	ci::Channel8uRef			mChannelBodyIndex;
	ci::Channel16uRef			mChannelDepth;
	ci::Channel16uRef			mChannelInfrared;
	ci::Surface8uRef			mSurfaceColor;

	float						mFrameRate;
	bool						mFullScreen;
	ci::params::InterfaceGlRef	mParams;
	int							mSend;
};


BatchassKinectApp::BatchassKinectApp() : mSpoutOut("VDRUI", app::getWindowSize())
{

	// Settings
	mVDSettings = VDSettings::create("VDRUI");
	// Uniform
	mVDUniforms = VDUniforms::create();
	// Params
	mVDParams = VDParams::create();
	// Animation
	mVDAnimation = VDAnimation::create(mVDSettings, mVDUniforms);
	// Mix
	mVDMix = VDMix::create(mVDSettings, mVDAnimation, mVDUniforms);
	// Session
	mVDSessionFacade = VDSessionFacade::createVDSession(mVDSettings, mVDAnimation, mVDUniforms, mVDMix)
		->setUniformValue(mVDUniforms->IDISPLAYMODE, VDDisplayMode::POST)
		->setupSession()
		->setupWSClient()
		->wsConnect()
		//->setupOSCReceiver()
		//->addOSCObserver(mVDSettings->mOSCDestinationHost, mVDSettings->mOSCDestinationPort)
		->addUIObserver(mVDSettings, mVDUniforms)
		->toggleUI()
		->setUniformValue(mVDUniforms->IBPM, 160.0f)
		->setUniformValue(mVDUniforms->IMOUSEX, 0.27710f)
		->setUniformValue(mVDUniforms->IMOUSEY, 0.5648f);

	mFadeInDelay = true;

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
	// UI
	mVDUI = VDUI::create(mVDSettings, mVDSessionFacade, mVDUniforms);
}

void BatchassKinectApp::toggleCursorVisibility(bool visible)
{
	if (visible)
	{
		showCursor();
	}
	else
	{
		hideCursor();
	}
}

void BatchassKinectApp::fileDrop(FileDropEvent event)
{
	mVDSessionFacade->fileDrop(event);
}

void BatchassKinectApp::mouseMove(MouseEvent event)
{
	if (!mVDSessionFacade->handleMouseMove(event)) {

	}
}

void BatchassKinectApp::mouseDown(MouseEvent event)
{

	if (!mVDSessionFacade->handleMouseDown(event)) {

	}
}

void BatchassKinectApp::mouseDrag(MouseEvent event)
{

	if (!mVDSessionFacade->handleMouseDrag(event)) {

	}
}

void BatchassKinectApp::mouseUp(MouseEvent event)
{

	if (!mVDSessionFacade->handleMouseUp(event)) {

	}
}

void BatchassKinectApp::keyDown(KeyEvent event)
{

	// warp editor did not handle the key, so handle it here
	if (!mVDSessionFacade->handleKeyDown(event)) {
		switch (event.getCode()) {
		case KeyEvent::KEY_F12:
			// quit the application
			quit();
			break;
		case KeyEvent::KEY_f:
			// toggle full screen
			setFullScreen(!isFullScreen());
			break;

		case KeyEvent::KEY_l:
			mVDSessionFacade->createWarp();
			break;
		}
	}
}

void BatchassKinectApp::keyUp(KeyEvent event)
{

	// let your application perform its keyUp handling here
	if (!mVDSessionFacade->handleKeyUp(event)) {
		
	}
}
void BatchassKinectApp::cleanup()
{
	CI_LOG_V("cleanup and save");
	ui::Shutdown();
	mVDSessionFacade->saveWarps();
	mVDSettings->save();
	CI_LOG_V("quit");
}

void BatchassKinectApp::update()
{
	mVDSessionFacade->setUniformValue(mVDUniforms->IFPS, getAverageFps());
	mVDSessionFacade->update();
	mFrameRate = getAverageFps();

	if (mFullScreen != isFullScreen()) {
		setFullScreen(mFullScreen);
		mFullScreen = isFullScreen();
	}
}


void BatchassKinectApp::resize()
{
	mVDUI->resize();
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
	//mSpoutOut.sendViewport();	

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
	// clear the window and set the drawing color to white
	/*gl::clear();
	gl::color(Color::white());
	if (mFadeInDelay) {
		mVDSettings->iAlpha = 0.0f;
		if (getElapsedFrames() > 10.0) {// mVDSessionFacade->getFadeInDelay()) {
			mFadeInDelay = false;
			timeline().apply(&mVDSettings->iAlpha, 0.0f, 1.0f, 1.5f, EaseInCubic());
		}
	}
	else {
		gl::setMatricesWindow(mVDParams->getFboWidth(), mVDParams->getFboHeight());
		
		int m = mVDSessionFacade->getUniformValue(mVDUniforms->IDISPLAYMODE);
		if (m == VDDisplayMode::MIXETTE) {
			gl::draw(mVDSessionFacade->buildRenderedMixetteTexture(0));
			mSpoutOut.sendTexture(mVDSessionFacade->buildRenderedMixetteTexture(0));
		}
		else if (m == VDDisplayMode::POST) {
			gl::draw(mVDSessionFacade->buildPostFboTexture());
			mSpoutOut.sendTexture(mVDSessionFacade->buildPostFboTexture());
		}
		else if (m == VDDisplayMode::FX) {
			gl::draw(mVDSessionFacade->buildFxFboTexture());
			mSpoutOut.sendTexture(mVDSessionFacade->buildFxFboTexture());
		}
		else {
			if (m < mVDSessionFacade->getFboShaderListSize()) {
				gl::draw(mVDSessionFacade->getFboShaderTexture(m));
				mSpoutOut.sendTexture(mVDSessionFacade->getFboShaderTexture(m));
			}
			else {
				gl::draw(mVDSessionFacade->buildRenderedMixetteTexture(0), Area(50, 50, mVDParams->getFboWidth() / 2, mVDParams->getFboHeight() / 2));
				gl::draw(mVDSessionFacade->buildPostFboTexture(), Area(mVDParams->getFboWidth() / 2, mVDParams->getFboHeight() / 2, mVDParams->getFboWidth(), mVDParams->getFboHeight()));
			}
			//gl::draw(mVDSession->getRenderedMixetteTexture(0), Area(0, 0, mVDSettings->mFboWidth, mVDSettings->mFboHeight));
			// ok gl::draw(mVDSession->getWarpFboTexture(), Area(0, 0, mVDSettings->mFboWidth, mVDSettings->mFboHeight));//getWindowBounds()
		}
	}	
	// imgui
	if (mVDSessionFacade->showUI()) {
		mVDUI->Run("UI", (int)getAverageFps());
		if (mVDUI->isReady()) {
		}
	}
	getWindow()->setTitle(toString((int)getAverageFps()) + " fps");*/
}
void prepareSettings(App::Settings *settings)
{
	settings->setWindowSize(1280, 720);
}
CINDER_APP(BatchassKinectApp, RendererGl(RendererGl::Options().msaa(8)),  prepareSettings)
