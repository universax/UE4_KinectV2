// Copyright 1998-2017 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "KinectManager.h"
#include "ModuleManager.h"
#include "Engine.h"
#include "KinectPluginBPLibrary.generated.h"

USTRUCT(BlueprintType)
struct FJoint
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Kinect|Joint")
		TArray<FVector> JointPosition;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Kinect|Joint")
		TArray<FRotator> JointRotation;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Kinect|Joint")
		TArray<bool> TrackingState;

	FJoint() {
		JointPosition.Reset(0);
		JointRotation.Reset(0);
		TrackingState.Reset(0);
	}
};

UENUM(BlueprintType)
enum class EJointType : uint8 {
	JointType_SpineBase UMETA(DisplayName = "SpineBase"),
	JointType_SpineMid UMETA(DisplayName = "SpineMid"),
	JointType_Neck UMETA(DisplayName = "Neck"),
	JointType_Head UMETA(DisplayName = "Head"),
	JointType_ShoulderLeft UMETA(DisplayName = "ShoulderLeft"),
	JointType_ElbowLeft UMETA(DisplayName = "ElbowLeft"),
	JointType_WristLeft UMETA(DisplayName = "WristLeft"),
	JointType_HandLeft UMETA(DisplayName = "HandLeft"),
	JointType_ShoulderRight UMETA(DisplayName = "ShoulderRight"),
	JointType_ElbowRight UMETA(DisplayName = "ElbowRight"),
	JointType_WristRight UMETA(DisplayName = "WristRight"),
	JointType_HandRight UMETA(DisplayName = "HandRight"),
	JointType_HipLeft UMETA(DisplayName = "HipLeft"),
	JointType_KneeLeft UMETA(DisplayName = "KneeLeft"),
	JointType_AnkleLeft UMETA(DisplayName = "AnkleLeft"),
	JointType_FootLeft UMETA(DisplayName = "FootLeft"),
	JointType_HipRight UMETA(DisplayName = "HipRight"),
	JointType_KneeRight UMETA(DisplayName = "KneeRight"),
	JointType_AnkleRight UMETA(DisplayName = "AnkleRight"),
	JointType_FootRight UMETA(DisplayName = "FootRight"),
	JointType_SpineShoulder UMETA(DisplayName = "SpineShoulder"),
	JointType_HandTipLeft UMETA(DisplayName = "HandTipLeft"),
	JointType_ThumbLeft UMETA(DisplayName = "ThumbLeft"),
	JointType_HandTipRight UMETA(DisplayName = "HandTipRight"),
	JointType_ThumbRight UMETA(DisplayName = "ThumbRight")
};

UCLASS(ClassGroup = Kinect, Blueprintable)
class UKinectPluginBPLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_UCLASS_BODY()


	UFUNCTION(BlueprintPure, meta = (HidePin = "WorldContextObject", DefaultToSelf = "WorldContextObject", DisplayName = "Create KinectPlugin Object From Blueprint", CompactNodeTitle = "Kinect", Keywords = "KinectPlugin"), Category = "Kinect")
		static UObject* NewObjectFromBlueprint(UObject* WorldContextObject, TSubclassOf<UObject> UC);

	UFUNCTION(BlueprintCallable, meta = (DisplayName = "Init Kinect", Keywords = "Init Kinect"), Category = "Kinect")
		void Init();

	UFUNCTION(BlueprintCallable, meta = (DisplayName = "Update Kinect", Keywords = "Update Kinect"), Category = "Kinect")
		bool Update();

	UFUNCTION(BlueprintCallable, meta = (DisplayName = "Close Kinect", Keywords = "Close Kinect"), Category = "Kinect")
		void Close();

	UFUNCTION(BlueprintCallable, meta = (DisplayName = "Get is anyone here.", Keywords = "Get is anyone here."), Category = "Kinect")
		bool IsAnyoneHere();

	UFUNCTION(BlueprintCallable, meta = (DisplayName = "Get Joint", Keywords = "Get Joint"), Category = "Kinect")
		FJoint GetJoint();

	UFUNCTION(BlueprintCallable, meta = (DisplayName = "Get Depth Points", Keywords = "Get Depth Points"), Category = "Kinect")
		TArray<FVector> GetDepthPoints();


	bool bAnyoneHere;

	//Generate Texture
	UPROPERTY(BlueprintReadOnly, Category = "Kinect|Camera Image Texture")
		FVector2D CameraImageTextureSize;

	UPROPERTY(BlueprintReadOnly, Category = "Kinect|Camera Image Texture")
		UTexture2D* CameraImageTexture;

	UFUNCTION(BlueprintCallable, meta = (DisplayName = "Update Camera Image Texture", Keywords = "Update Camera Image Texture"), Category = "Kinect|Camera Image Texture")
		void UpdateCameraImageTexture();

	UFUNCTION(BlueprintCallable, meta = (DisplayName = "Get Camera Image With Texture", Keywords = "Get Camera Image With Texture"), Category = "Kinect|Camera Image Texture")
		UTexture2D* GetCameraImageTexture();


private:
	//Kinect
	KinectManager kinect;

	//Generate Texture
	//DynamnicTexture
	TArray<FColor> CameraImageData;
	FUpdateTextureRegion2D * CameraImageUpdateTextureRegion;
	bool DidDynamicTextureInit;

	void InitDynamicTextureResorces();
	void UpdateTextureRegions(
		UTexture2D* Texture,
		int32 MipIndex,
		uint32 NumRegions,
		FUpdateTextureRegion2D* Regions,
		uint32 SrcPitch,
		uint32 SrcBpp,
		uint8* SrcData,
		bool bFreeData);
};

