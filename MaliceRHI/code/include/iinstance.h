#pragma once

#include <string>
#include <iostream>


// Forward declarations
class VulkanInstance;

// Instance interface
class IInstance
{
protected:
	/// Class properties ///

	// Arbitrary name of the instance.
	std::string instanceName;

public:
	// Class destructor
	virtual ~IInstance() = default;


	/// Lifetime methods ///

	virtual void Create(const char* _instanceName) = 0;
	virtual void Destroy() = 0;


	/// Retrieving the backend ///

	virtual VulkanInstance& API_Vulkan() { throw std::runtime_error("Bad API call : object is not a VulkanInstance."); }


	/// Class specific methods ///

	const char* GetInstanceName() { return instanceName.c_str(); }
};