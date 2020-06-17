#include "input-button-mapping.h"

void cnButtonMapping_Clear(CnButtonMapping* mapping)
{
	CN_ASSERT_NOT_NULL(mapping);
	for (uint32_t i = 0; i < CN_MAX_BUTTON_MAPPINGS; ++i) {
		mapping->physical[i] = CN_INVALID_PHYSICAL_BUTTON;
		mapping->logical[i] = CN_INVALID_DIGITAL_BUTTON;
	}
	mapping->numMappings = 0;
}

bool cnButtonMapping_IsMapped(CnButtonMapping* mapping, CnPhysicalButtonId buttonId)
{
	CN_ASSERT_NOT_NULL(mapping);
	CN_ASSERT(buttonId != CN_INVALID_PHYSICAL_BUTTON,
		"Looking up logical button for an invalid physical button.");

	return cnButtonMapping_LookUp(mapping, buttonId) != CN_INVALID_DIGITAL_BUTTON;
}

void cnButtonMapping_Map(CnButtonMapping* mapping, CnPhysicalButtonId source, CnDigitalButton* button)
{
	CN_ASSERT_NOT_NULL(mapping);
	CN_ASSERT_NOT_NULL(button);

	CN_ASSERT(mapping->numMappings < CN_MAX_BUTTON_MAPPINGS, "Too many buttons mappings created.");

	mapping->physical[mapping->numMappings] = source;
	mapping->logical[mapping->numMappings] = button;

	++mapping->numMappings;
}

CnDigitalButton* cnButtonMapping_LookUp(CnButtonMapping* mapping, CnPhysicalButtonId source)
{
	for (uint32_t i = 0; i < mapping->numMappings; ++i) {
		CN_ASSERT(mapping->physical[i] != CN_INVALID_PHYSICAL_BUTTON,
			"Invalid physical button mapping found.");
		if (mapping->physical[i] == source) {
			return mapping->logical[i];
		}
	}
}
