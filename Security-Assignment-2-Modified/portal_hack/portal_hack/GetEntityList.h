#pragma once
#define MAKE_PAD(size) STR _MERGE(_pad,__COUNTER__)[size]
#define DEFINE_MEMBER_N(type, name, offset) struct {unsigned char MAKE_PAD(offset); type name;}

struct vec3 { float x, y, z; };

// Adapted from https://github.com/ValveSoftware/source-sdk-2013/blob/0d8dceea4310fde5706b3ce1c70609d72a38efdf/mp/src/public/icliententitylist.h#L42
class IClientEntityList
{
public:
	// Get IClientNetworkable interface for specified entity
	virtual int * GetClientNetworkable(int entnum) = 0;
	virtual int* GetClientNetworkableFromHandle(int hEnt) = 0;
	virtual int* GetClientUnknownFromHandle(int hEnt) = 0;

	// NOTE: This function is only a convenience wrapper.
	// It returns GetClientNetworkable( entnum )->GetIClientEntity().
	virtual void* GetClientEntity(int entnum) = 0;
	virtual void* GetClientEntityFromHandle(int hEnt) = 0;

	// Returns number of entities currently in use
	virtual int					NumberOfEntities(bool bIncludeNonNetworkable) = 0;

	// Returns highest index actually used
	virtual int					GetHighestEntityIndex(void) = 0;

	// Sizes entity list to specified size
	virtual void				SetMaxEntities(int maxents) = 0;
	virtual int					GetMaxEntities() = 0;
};