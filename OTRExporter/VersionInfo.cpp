#include "VersionInfo.h"
#include <libultraship/bridge.h>
#include "SohResourceType.h"

std::map<uint32_t, uint32_t> resourceVersions;

void InitVersionInfo()
{
	resourceVersions = std::map<uint32_t, uint32_t> {
	{ static_cast<uint32_t>(SohResourceType::SOH_Animation), 0 },
	{ static_cast<uint32_t>(LUS::ResourceType::Texture), 0 },
	{ static_cast<uint32_t>(SohResourceType::SOH_PlayerAnimation), 0 },
	{ static_cast<uint32_t>(LUS::ResourceType::DisplayList), 0 },
	{ static_cast<uint32_t>(SohResourceType::SOH_Room), 0 },
	{ static_cast<uint32_t>(SohResourceType::SOH_CollisionHeader), 0 },
	{ static_cast<uint32_t>(SohResourceType::SOH_Skeleton), 0 },
	{ static_cast<uint32_t>(SohResourceType::SOH_SkeletonLimb), 0 },
	{ static_cast<uint32_t>(LUS::ResourceType::Matrix), 0 },
	{ static_cast<uint32_t>(SohResourceType::SOH_Path), 0 },
	{ static_cast<uint32_t>(LUS::ResourceType::Vertex), 0 },
	{ static_cast<uint32_t>(SohResourceType::SOH_Cutscene), 0 },
	{ static_cast<uint32_t>(LUS::ResourceType::Array), 0 },
	{ static_cast<uint32_t>(SohResourceType::SOH_Text), 0 },
	{ static_cast<uint32_t>(LUS::ResourceType::Blob), 0 },
	};
}
