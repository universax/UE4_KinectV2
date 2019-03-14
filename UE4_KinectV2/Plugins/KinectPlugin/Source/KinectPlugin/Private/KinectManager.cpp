#include "KinectManager.h"

HRESULT KinectManager::init() {
	//-----------------------------------------------
	//Kinect											 
	//-----------------------------------------------
	HRESULT hr = S_FALSE;
	if (isRunning)
	{
		return hr;
	}

	hr = GetDefaultKinectSensor(&kinect);
	if (FAILED(hr)) {
		// cerr << "ERROR: GetDefaultKinectSensor" << endl;
		return hr;
	}
	hr = kinect->Open();
	if (FAILED(hr))
	{
		// cerr << "ERROR: IKinectSensor::Open()" << endl;
		return hr;
	}
	isRunning = true;

	//-----------------------------------------------
	//Coordinate mapper											 
	//-----------------------------------------------
	hr = kinect->get_CoordinateMapper(&pCoordinateMapper);
	if (FAILED(hr))
	{
		// cerr << "ERROR: ICoordinateMapper::get_CoordinateMapper()" << endl;
		return hr;
	}


	//-----------------------------------------------
	//Color											 
	//-----------------------------------------------
	IColorFrameSource* colorFrameSource;
	hr = kinect->get_ColorFrameSource(&colorFrameSource);
	if (FAILED(hr))
	{
		// cerr << "ERROR: IColorFrameSource::get_ColorFrameSource()" << endl;
		return hr;
	}
	colorFrameSource->OpenReader(&colorFrameReader);

	IFrameDescription* colorFrameDescription;
	hr = colorFrameSource->CreateFrameDescription(ColorImageFormat::ColorImageFormat_Bgra, &colorFrameDescription);
	if (FAILED(hr))
	{
		// cerr << "ERROR: IFrameDescription::CreateFrameDescription()" << endl;
		return hr;
	}
	colorFrameSource->Release();


	colorFrameDescription->get_Width(&colorWidth);
	colorFrameDescription->get_Height(&colorHeight);
	colorFrameDescription->get_BytesPerPixel(&colorBytesPerPixel);
	colorFrameDescription->Release();

	colorBuffer.resize(colorWidth*colorHeight*colorBytesPerPixel);


	//-----------------------------------------------
	//Depth											 
	//-----------------------------------------------
	depthBuffer.resize(bufferSize);
	IDepthFrameSource* pDepthSource;
	hr = kinect->get_DepthFrameSource(&pDepthSource);
	if (FAILED(hr))
	{
		// cerr << "ERROR: IDepthFrameSource::get_DepthFrameSource()" << endl;
		return hr;
	}
	hr = pDepthSource->OpenReader(&pDepthReader);
	if (FAILED(hr))
	{
		// cerr << "ERROR: IDepthFrameReader::OpenReader()" << endl;
		return hr;
	}
	pDepthSource->Release();
	isFrameNew = false;

	//-----------------------------------------------
	//IR											 
	//-----------------------------------------------
	IInfraredFrameSource* infraredFrameSource;
	hr = kinect->get_InfraredFrameSource(&infraredFrameSource);
	if (FAILED(hr))
	{
		// cerr << "ERROR: IInfraredFrameSource::get_InfraredFrameSource()" << endl;
		return hr;
	}
	hr = infraredFrameSource->OpenReader(&infraredFrameReader);
	if (FAILED(hr))
	{
		// cerr << "ERROR: IInfraredFrameReader::OpenReader()" << endl;
		return hr;
	}

	IFrameDescription* infraredFrameDescription;
	hr = infraredFrameSource->get_FrameDescription(&infraredFrameDescription);
	if (FAILED(hr))
	{
		// cerr << "ERROR: IFrameDescription::get_FrameDescription()" << endl;
		return hr;
	}
	infraredFrameSource->Release();
	hr = infraredFrameDescription->get_Width(&infraredFrameWidth);
	if (FAILED(hr))
	{
		// cerr << "ERROR: IFrameDescription::get_Width()" << endl;
		return hr;
	}
	hr = infraredFrameDescription->get_Height(&infraredFrameHeight);
	if (FAILED(hr))
	{
		// cerr << "ERROR: IFrameDescription::get_Height()" << endl;
		return hr;
	}
	infraredFrameDescription->Release();
	infraredBuffer.resize(infraredFrameWidth * infraredFrameHeight);

	//-----------------------------------------------
	//BodyIndex											 
	//-----------------------------------------------
	IBodyIndexFrameSource* bodyIndexFrameSource;
	hr = kinect->get_BodyIndexFrameSource(&bodyIndexFrameSource);
	if (FAILED(hr))
	{
		// cerr << "ERROR: IBodyIndexFrameSource::get_BodyIndexFrameSource()" << endl;
		return hr;
	}
	hr = bodyIndexFrameSource->OpenReader(&pBodyIndexFrameReader);
	if (FAILED(hr))
	{
		// cerr << "ERROR: IBodyIndexFrameReader::OpenReader()" << endl;
		return hr;
	}
	bodyIndexFrameSource->Release();
	bodyIndexBuffer.resize(bufferSize);


	//-----------------------------------------------
	//Body											 
	//-----------------------------------------------
	IBodyFrameSource* bodyFrameSource;
	hr = kinect->get_BodyFrameSource(&bodyFrameSource);
	if (FAILED(hr))
	{
		// cerr << "ERROR: IBodyFrameSource::get_BodyFrameSource()" << endl;
		return hr;
	}
	hr = bodyFrameSource->OpenReader(&bodyFrameReader);
	if (FAILED(hr))
	{
		// cerr << "ERROR: IBodyFrameReader::OpenReader()" << endl;
		return hr;
	}
	bodyFrameSource->Release();
	return hr;
}


bool KinectManager::update() {
	if (!isRunning)
	{
		return false;
	}


	isFrameNew = updateDepthFrame();
	if (isFrameNew)
	{
		//ColorâÊëúÇ…ëŒâûÇµÇΩCameraPointîzóÒê∂ê¨
		//mapColorSpaceToCameraSpace(camSpaceBuf);
		updateInfraredFrame();
		updateBodyFrame();
	}
	else {
		// cerr << "ERROR: isFrameNew = false" << endl;
	}

	return isFrameNew;
}

HRESULT KinectManager::close() {
	HRESULT hr = S_FALSE;
	if (!isRunning)
	{
		return hr;
	}

	hr = kinect->Close();
	if (FAILED(hr))
	{
		// cerr << "ERROR: IKinectSensor::Close()" << endl;
		return hr;
	}

	kinect->Release();
	pCoordinateMapper->Release();
	colorFrameReader->Release();
	pDepthReader->Release();
	infraredFrameReader->Release();
	pBodyIndexFrameReader->Release();
	bodyFrameReader->Release();

	isRunning = false;
	return hr;
}

//--------------------------------------------------------------------------------------------
void KinectManager::getDepthBuf(vector<UINT16> &output)
{
	output.clear();

	copy(depthBuffer.begin(), depthBuffer.end(), back_inserter(output));
}

void KinectManager::getInfraredBuf(vector<UINT16>& output)
{
	output.clear();
	copy(infraredBuffer.begin(), infraredBuffer.end(), back_inserter(output));
}

IBody* KinectManager::getBodies(int index)
{
	return bodies[index];
}

void KinectManager::getDepthPointsWithCamSpacePoint(vector<CameraSpacePoint>& camSpacePoints)
{
	camSpacePoints.resize(depthBuffer.size());
	pCoordinateMapper->MapDepthFrameToCameraSpace(depthBuffer.size(), &depthBuffer[0], depthBuffer.size(), &camSpacePoints[0]);
}



//--------------------------------------------------------------------------------------------
bool KinectManager::updateDepthFrame() {
	HRESULT hr = S_FALSE;
	IDepthFrame* pDepthFrame;
	if (pDepthReader == nullptr)
	{
		return false;
	}
	hr = pDepthReader->AcquireLatestFrame(&pDepthFrame);
	if (SUCCEEDED(hr))
	{
		pDepthFrame->CopyFrameDataToArray(depthBuffer.size(), &depthBuffer[0]);
		pDepthFrame->Release();
		return true;
	}
	return false;
}

bool KinectManager::updateColorFrame() {
	HRESULT hr = S_FALSE;
	IColorFrame* colorFrame;
	hr = colorFrameReader->AcquireLatestFrame(&colorFrame);
	if (SUCCEEDED(hr))
	{
		colorFrame->CopyConvertedFrameDataToArray(colorBuffer.size(), &colorBuffer[0], ColorImageFormat::ColorImageFormat_Bgra);
		colorFrame->Release();
		return true;
	}
	return false;
}

bool KinectManager::updateInfraredFrame()
{
	HRESULT hr = S_FALSE;
	IInfraredFrame* pInfraredFrame;
	hr = infraredFrameReader->AcquireLatestFrame(&pInfraredFrame);
	if (SUCCEEDED(hr))
	{
		pInfraredFrame->CopyFrameDataToArray(infraredBuffer.size(), &infraredBuffer[0]);
		pInfraredFrame->Release();
		return true;
	}
	return false;
}

bool KinectManager::updateBodyIndexFrame() {
	HRESULT hr = S_FALSE;
	IBodyIndexFrame* pBodyIndexFrame;
	hr = pBodyIndexFrameReader->AcquireLatestFrame(&pBodyIndexFrame);
	if (SUCCEEDED(hr))
	{
		pBodyIndexFrame->CopyFrameDataToArray(bodyIndexBuffer.size(), &bodyIndexBuffer[0]);
		pBodyIndexFrame->Release();
		return true;
	}
	return false;
}

bool KinectManager::updateBodyFrame()
{
	IBodyFrame* bodyFrame;
	HRESULT hr = bodyFrameReader->AcquireLatestFrame(&bodyFrame);
	if (FAILED(hr))
	{
		// cerr << "ERROR: IBodyFrameReader::AcquireLatestFrame()" << endl;
		return false;
	}

	for (auto &b : bodies) {
		if (b != nullptr)
		{
			b->Release();
			b = nullptr;
		}
	}

	hr = bodyFrame->GetAndRefreshBodyData(6, &bodies[0]);
	if (FAILED(hr))
	{
		// cerr << "ERROR: IBodyFrame::GetAndRefreshBodyData()" << endl;
		return false;
	}
	bodyFrame->Release();
	return true;
}
