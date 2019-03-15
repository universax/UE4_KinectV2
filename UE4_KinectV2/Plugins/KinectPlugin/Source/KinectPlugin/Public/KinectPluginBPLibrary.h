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
	//UPROPERTY(BlueprintReadOnly, Category = "Kinect|Camera Image Texture")
	//	FVector2D CameraImageTextureSize_IR;

	//UPROPERTY(BlueprintReadOnly, Category = "Kinect|Camera Image Texture")
	//	UTexture2D* CameraImageTexture_IR;

	UFUNCTION(BlueprintCallable, meta = (DisplayName = "Update Camera Texture", Keywords = "Update Camera Texture"), Category = "Kinect|Camera")
		void UpdateCameraTexture();

	UFUNCTION(BlueprintCallable, meta = (DisplayName = "Get Camera IR Texture", Keywords = "Get Camera IR Texture"), Category = "Kinect|Camera")
		UTexture2D* GetCameraImageTexture_IR();

	UFUNCTION(BlueprintCallable, meta = (DisplayName = "Get Camera Color Texture", Keywords = "Get Camera Color Texture"), Category = "Kinect|Camera")
		UTexture2D* GetCameraImageTexture_Color();

	UFUNCTION(BlueprintCallable, meta = (DisplayName = "Get Camera Depth Texture", Keywords = "Get Camera Depth Texture"), Category = "Kinect|Camera")
		UTexture2D* GetCameraImageTexture_Depth();


private:
	//Kinect
	KinectManager kinect;

	//Generate Texture
	//DynamnicTexture
	// IR
	FVector2D CameraImageTextureSize_IR;
	UTexture2D* CameraImageTexture_IR;
	TArray<FColor> CameraImageData_IR;
	FUpdateTextureRegion2D * CameraImageUpdateTextureRegion_IR;
	// Color
	FVector2D CameraImageTextureSize_Color;
	UTexture2D* CameraImageTexture_Color;
	TArray<FColor> CameraImageData_Color;
	FUpdateTextureRegion2D * CameraImageUpdateTextureRegion_Color;
	// Depth
	FVector2D CameraImageTextureSize_Depth;
	UTexture2D* CameraImageTexture_Depth;
	TArray<FColor> CameraImageData_Depth;
	FUpdateTextureRegion2D * CameraImageUpdateTextureRegion_Depth;

	void UpdateCameraTexture(FVector2D &CameraImageTextureSize,
		UTexture2D* CameraImageTexture, 
		TArray<FColor> &CameraImageData,
		FUpdateTextureRegion2D* CameraImageUpdateTextureRegion);


	bool DidDynamicTextureInit;

	void InitDynamicTextureResorces();

	void InitDynamicTextureResorces(FVector2D &ImgTextureSize, 
		UTexture2D* CameraImageTexture,
		TArray<FColor> &CameraImageData,
		FUpdateTextureRegion2D* CameraImageUpdateTextureRegion);

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

