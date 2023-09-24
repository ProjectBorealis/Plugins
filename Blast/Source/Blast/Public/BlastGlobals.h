#pragma once

#include "CoreMinimal.h"

#include "NvCTypes.h"

DECLARE_LOG_CATEGORY_EXTERN(LogBlast, Log, All);

NvcQuat ToNvQuat(const FQuat& Quat)
{
	return { static_cast<float>(Quat.X), static_cast<float>(Quat.Y), static_cast<float>(Quat.Z), static_cast<float>(Quat.W) };
}

NvcVec3 ToNvVector(const FVector& Vec)
{
	return { static_cast<float>(Vec.X), static_cast<float>(Vec.Y), static_cast<float>(Vec.Z) };
}

void ToNvPlane4(const FPlane& Plane, float OutPlane[4])
{
	OutPlane[0] =  static_cast<float>(Plane.X);
	OutPlane[1] =  static_cast<float>(Plane.Y);
	OutPlane[2] =  static_cast<float>(Plane.Z);
	OutPlane[3] =  -static_cast<float>(Plane.W);
}

FVector FromNvVector(const NvcVec3& Vec)
{
	return { static_cast<double>(Vec.x), static_cast<double>(Vec.y), static_cast<double>(Vec.z) };
}

FPlane FromNvPlane4(float Plane[4])
{
	return { static_cast<double>(Plane[0]), static_cast<double>(Plane[1]), static_cast<double>(Plane[2]), -static_cast<double>(Plane[3]) };
}
