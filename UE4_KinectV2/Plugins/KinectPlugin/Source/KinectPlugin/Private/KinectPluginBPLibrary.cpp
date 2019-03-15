// Copyright 1998-2017 Epic Games, Inc. All Rights Reserved.

#include "KinectPluginBPLibrary.h"
#include "KinectPlugin.h"


DEFINE_LOG_CATEGORY(Kinect);

UKinectPluginBPLibrary::UKinectPluginBPLibrary(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{

}

UObject* UKinectPluginBPLibrary::NewObjectFromBlueprint(UObject* WorldContextObject, TSubclassOf<UObject> UC)
{
	//UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject);
	return NewObject<UKinectPluginBPLibrary>(UC);
	//return StaticConstructObject(UC);
}

void UKinectPluginBPLibrary::Init()
{
	//Flag
	bAnyoneHere = false;

	if (kinect.getIsRunning())
	{
		return;
	}
	HRESULT hr = kinect.init();
	if (SUCCEEDED(hr))
	{
		UE_LOG(Kinect, Warning, TEXT("UKinectPluginBPLibrary: Sccess Init Kinect"));
	}
}

bool UKinectPluginBPLibrary::Update()
{
	return kinect.update();
}

void UKinectPluginBPLibrary::Close()
{
	if (!kinect.getIsRunning())
	{
		return;
	}
	HRESULT hr = kinect.close();
	if (SUCCEEDED(hr))
	{
		UE_LOG(Kinect, Warning, TEXT("UKinectPluginBPLibrary: Sccess Close Kinect"));
	}
}

bool UKinectPluginBPLibrary::IsAnyoneHere()
{
	return bAnyoneHere;
}

FJoint UKinectPluginBPLibrary::GetJoint()
{
	FJoint targetJoint;


	//真ん中かつ一番手前の人のインデックスを探す
	int centerBodyIndex = 0;
	int trackingCount = 0;
	float minDistance = 9999;
	for (int i = 0; i < BODY_COUNT; i++)
	{
		IBody* body = kinect.getBodies(i);
		if (body == nullptr)
		{
			continue;
		}

		BOOLEAN isTracked;
		body->get_IsTracked(&isTracked);
		if (isTracked)
		{
			trackingCount += 1;

			//頭の位置とかで適当にやる
			Joint joints[JointType::JointType_Count];
			body->GetJoints(JointType::JointType_Count, joints);
			Joint headJoint = joints[JointType::JointType_Head];
			if (headJoint.TrackingState == TrackingState::TrackingState_Tracked)
			{
				float dist = sqrtf(headJoint.Position.X*headJoint.Position.X + headJoint.Position.Y*headJoint.Position.Y);
				if (dist < minDistance)
				{
					centerBodyIndex = i;
					minDistance = dist;
				}
			}
		}
	}


	//誰もいなかったらさよなら
	if (trackingCount == 0)
	{
		//UE_LOG(Kinect, Warning, TEXT("UKinectPluginBPLibrary: No one here."));
		bAnyoneHere = false;
		return targetJoint;
	}

	//誰かしらいる
	bAnyoneHere = true;
	//UE_LOG(Kinect, Warning, TEXT("UKinectPluginBPLibrary: tracking %d"), centerBodyIndex);

	//一番真ん中かつ一番手前の人のJointを返す
	IBody* centerMan = kinect.getBodies(centerBodyIndex);
	if (centerMan != nullptr)
	{
		Joint joints[JointType::JointType_Count];
		centerMan->GetJoints(JointType::JointType_Count, joints);
		for (auto &j : joints) {
			//Position
			FVector pos(j.Position.X, j.Position.Z, j.Position.Y);
			targetJoint.JointPosition.Add(pos);

			//TrackingState
			bool bTracking = (j.TrackingState == TrackingState::TrackingState_Tracked);
			targetJoint.TrackingState.Add(bTracking);
		}

		JointOrientation jointOrientations[JointType::JointType_Count];
		centerMan->GetJointOrientations(JointType::JointType_Count, jointOrientations);
		for (auto &jo : jointOrientations)
		{
			Vector4 rotate = jo.Orientation;
			FVector4 fRotate(rotate.x, rotate.y, rotate.z, rotate.w);
			FRotator rotator = fRotate.Rotation();
			targetJoint.JointRotation.Add(rotator);
		}
	}

	return targetJoint;
}


TArray<FVector> UKinectPluginBPLibrary::GetDepthPoints() {
	//Get Depth buffer
	vector<UINT16> buf;
	kinect.getDepthBuf(buf);

	//Map to CameraSpacePoint
	vector<CameraSpacePoint> camPoints;
	kinect.getDepthPointsWithCamSpacePoint(camPoints);

	TArray<FVector> points;
	for (int i = 0; i < camPoints.size(); i++)
	{
		FVector p(camPoints[i].X, camPoints[i].Y, camPoints[i].Z);
		points.Add(p);
	}


	return points;
}


//--------------------------------------------------------------------------------------
//	Dynamic Texture
//--------------------------------------------------------------------------------------
//Texture生成周りの処理
void UKinectPluginBPLibrary::InitDynamicTextureResorces()
{
	// IR
	CameraImageTextureSize_IR.X = 512;
	CameraImageTextureSize_IR.Y = 424;
	InitDynamicTextureResorces(CameraImageTextureSize_IR, CameraImageTexture_IR, CameraImageData_IR, CameraImageUpdateTextureRegion_IR);

	// Color
	CameraImageTextureSize_Color.X = 1920;
	CameraImageTextureSize_Color.Y = 1080;
	InitDynamicTextureResorces(CameraImageTextureSize_Color, CameraImageTexture_Color , CameraImageData_Color, CameraImageUpdateTextureRegion_Color);

	// Depth
	CameraImageTextureSize_Depth.X = 512;
	CameraImageTextureSize_Depth.Y = 424;
	InitDynamicTextureResorces(CameraImageTextureSize_Depth, CameraImageTexture_Depth, CameraImageData_Depth, CameraImageUpdateTextureRegion_Depth);



}

void UKinectPluginBPLibrary::InitDynamicTextureResorces(FVector2D & ImgTextureSize, UTexture2D * CameraImageTexture, TArray<FColor>& CameraImageData, FUpdateTextureRegion2D * CameraImageUpdateTextureRegion)
{
#undef UpdateResource	//winbase.hのマクロ定義と衝突するので回避
	CameraImageTexture = UTexture2D::CreateTransient(ImgTextureSize.X, ImgTextureSize.Y, PF_B8G8R8A8);
	CameraImageTexture->UpdateResource();
	CameraImageData.Init(FColor(0, 0, 0, 255), ImgTextureSize.X * ImgTextureSize.Y);

	CameraImageUpdateTextureRegion = new FUpdateTextureRegion2D(0, 0, 0, 0, ImgTextureSize.X, ImgTextureSize.Y);
}

void UKinectPluginBPLibrary::UpdateTextureRegions(UTexture2D * Texture, int32 MipIndex, uint32 NumRegions, FUpdateTextureRegion2D * Regions, uint32 SrcPitch, uint32 SrcBpp, uint8 * SrcData, bool bFreeData)
{
	if (Texture->Resource)
	{
		struct FUpdateTextureRegionsData
		{
			FTexture2DResource* Texture2DResource;
			int32 MipIndex;
			uint32 NumRegions;
			FUpdateTextureRegion2D* Regions;
			uint32 SrcPitch;
			uint32 SrcBpp;
			uint8* SrcData;
		};

		FUpdateTextureRegionsData* RegionData = new FUpdateTextureRegionsData;

		RegionData->Texture2DResource = (FTexture2DResource*)Texture->Resource;
		RegionData->MipIndex = MipIndex;
		RegionData->NumRegions = NumRegions;
		RegionData->Regions = Regions;
		RegionData->SrcPitch = SrcPitch;
		RegionData->SrcBpp = SrcBpp;
		RegionData->SrcData = SrcData;

		ENQUEUE_UNIQUE_RENDER_COMMAND_TWOPARAMETER(
			UpdateTextureRegionsData,
			FUpdateTextureRegionsData*, RegionData, RegionData,
			bool, bFreeData, bFreeData,
			{
				for (uint32 RegionIndex = 0; RegionIndex < RegionData->NumRegions; ++RegionIndex)
				{
					int32 CurrentFirstMip = RegionData->Texture2DResource->GetCurrentFirstMip();
					if (RegionData->MipIndex >= CurrentFirstMip)
					{
						RHIUpdateTexture2D(
							RegionData->Texture2DResource->GetTexture2DRHI(),
							RegionData->MipIndex - CurrentFirstMip,
							RegionData->Regions[RegionIndex],
							RegionData->SrcPitch,
							RegionData->SrcData
							+ RegionData->Regions[RegionIndex].SrcY * RegionData->SrcPitch
							+ RegionData->Regions[RegionIndex].SrcX * RegionData->SrcBpp
						);
					}
				}
		if (bFreeData)
		{
			FMemory::Free(RegionData->Regions);
			FMemory::Free(RegionData->SrcData);
		}
		delete RegionData;
			});
	}
}

void UKinectPluginBPLibrary::UpdateCameraTexture()
{
	//初回だけテクスチャ周りをイニシャライズしておく
	if (!DidDynamicTextureInit)
	{
		InitDynamicTextureResorces();
		DidDynamicTextureInit = true;
	}

	//Get kinect Camera Images
	// IR
	vector<UINT16> buf_IR;
	kinect.getInfraredBuf(buf_IR);
	for (int i = 0; i < buf_IR.size(); i++)
	{
		BYTE b = 255 * buf_IR[i] * 30 / (255 * 255);

		CameraImageData_IR[i].R = b;	//x
		CameraImageData_IR[i].G = b;	//y
		CameraImageData_IR[i].B = b;	//z
	}
	UpdateCameraTexture(CameraImageTextureSize_IR, CameraImageTexture_IR, CameraImageData_IR, CameraImageUpdateTextureRegion_IR);

	// Color
	vector<BYTE> buf_Color;
	kinect.getColorBur(buf_Color);
	int j = 0;
	for (int i = 0; i < buf_Color.size(); i+=4)
	{
		BYTE r = buf_Color[i];
		BYTE g = buf_Color[i+1];
		BYTE b = buf_Color[i+2];

		CameraImageData_Color[j].R = r;	//x
		CameraImageData_Color[j].G = g;	//y
		CameraImageData_Color[j].B = b;	//z

		j++;
	}
	UpdateCameraTexture(CameraImageTextureSize_Color, CameraImageTexture_Color, CameraImageData_Color, CameraImageUpdateTextureRegion_Color);

	// Depth
	vector<UINT16> buf_Depth;
	kinect.getDepthBuf(buf_Depth);
	for (int i = 0; i < buf_Depth.size(); i++)
	{
		BYTE b = 255 * buf_Depth[i] * 30 / (255 * 255);

		CameraImageData_Depth[i].R = b;	//x
		CameraImageData_Depth[i].G = b;	//y
		CameraImageData_Depth[i].B = b;	//z
	}
	UpdateCameraTexture(CameraImageTextureSize_Depth, CameraImageTexture_Depth, CameraImageData_Depth, CameraImageUpdateTextureRegion_Depth);
}

void UKinectPluginBPLibrary::UpdateCameraTexture(FVector2D &CameraImageTextureSize, UTexture2D* CameraImageTexture, TArray<FColor> &CameraImageData, FUpdateTextureRegion2D* CameraImageUpdateTextureRegion)
{
	//Nullチェック
	if (!CameraImageTexture)
	{
		//UE_LOG(PCLPlugin, Error, TEXT("ERROR: PointCloudTexture is null"));
		return;
	}

	UpdateTextureRegions(
		CameraImageTexture,
		(int32)0,
		(uint32)1,
		CameraImageUpdateTextureRegion,
		(uint32)4 * CameraImageTextureSize.X,
		(uint32)4,
		(uint8*)CameraImageData.GetData(),
		false
	);
}

UTexture2D * UKinectPluginBPLibrary::GetCameraImageTexture_IR()
{
	if (!CameraImageTexture_IR)
	{
		UE_LOG(Kinect, Warning, TEXT("ERROR: IR DynamicTexture is null"));
	}

	return CameraImageTexture_IR;
}

UTexture2D * UKinectPluginBPLibrary::GetCameraImageTexture_Color()
{
	if (!CameraImageTexture_Color)
	{
		UE_LOG(Kinect, Warning, TEXT("ERROR: Color DynamicTexture is null"));
	}

	return CameraImageTexture_Color;
}

UTexture2D * UKinectPluginBPLibrary::GetCameraImageTexture_Depth()
{
	if (!CameraImageTexture_Depth)
	{
		UE_LOG(Kinect, Warning, TEXT("ERROR: Depth DynamicTexture is null"));
	}

	return CameraImageTexture_Depth;
}
