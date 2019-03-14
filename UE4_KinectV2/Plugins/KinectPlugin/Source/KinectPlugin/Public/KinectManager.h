#pragma once
#include "Singleton.h"

#include <stdio.h>
#include <iostream>
#include <vector>
#include <fstream>
#include <sstream>
#include <string>
#include <memory>
#include <algorithm>
#include <iterator>
#include <cmath>

#if PLATFORM_WINDOWS
#include "AllowWindowsPlatformTypes.h"
#include <Kinect.h>
#endif

using namespace std;
class KinectManager
{
public:
	KinectManager() {
		isRunning = false;
		//cout << "KinectManager Createed!!!!!!!!!!!!" << endl;
	};
	~KinectManager() {};

	HRESULT init();
	bool update();
	HRESULT close();
	inline bool getIsFrameNew() { return isFrameNew; }
	void getDepthBuf(vector<UINT16> &output);
	void getInfraredBuf(vector<UINT16> &output);
	IBody* getBodies(int index);
	bool getIsRunning() { return isRunning; }
	void getDepthPointsWithCamSpacePoint(vector<CameraSpacePoint> &camSpacePoints);
	
private:
	//friend class Singleton < KinectManager > ;
	

	//------------------------------------------------------------
	//KinectŽü‚è‚ÌŠî–{‹@”\
	//------------------------------
	//Kinect
	IKinectSensor* kinect;
	bool isFrameNew;
	bool isRunning = false;

	//------------------------------
	//CoordinateMapper
	ICoordinateMapper* pCoordinateMapper;

	//------------------------------
	//Color
	IColorFrameReader* colorFrameReader;
	int colorWidth;
	int colorHeight;
	unsigned int colorBytesPerPixel;
	vector<BYTE>colorBuffer;
	bool updateColorFrame();
	
	//------------------------------
	//Depth
	IDepthFrameReader* pDepthReader;
	vector<UINT16> depthBuffer;
	bool updateDepthFrame();
	unsigned int bufferSize = 512 * 424;
	

	//------------------------------
	//IR
	IInfraredFrameReader* infraredFrameReader;
	int infraredFrameWidth, infraredFrameHeight;
	vector<UINT16> infraredBuffer;
	bool updateInfraredFrame();

	//------------------------------
	//Body Index
	IBodyIndexFrameReader* pBodyIndexFrameReader;
	vector<BYTE>bodyIndexBuffer;
	bool updateBodyIndexFrame();

	//------------------------------
	//Body
	IBodyFrameReader* bodyFrameReader;
	IBody* bodies[BODY_COUNT];
	bool updateBodyFrame();
};
//
#if PLATFORM_WINDOWS
#include "HideWindowsPlatformTypes.h"
#endif