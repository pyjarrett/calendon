#include "input-button-mapping.h"

/**
 * Unmaps all buttons.
 */
void cnButtonMapping_Clear(CnButtonMapping* mapping)
{
	CN_ASSERT_PTR(mapping);
	for (uint32_t i = 0; i < CN_MAX_BUTTON_MAPPINGS; ++i) {
		mapping->physical[i] = CN_PHYSICAL_BUTTON_ID_INVALID;
		mapping->logical[i] = CN_DIGITAL_BUTTON_INVALID;
	}
	mapping->numMappings = 0;
}

/**
 * Determine if a button is mapped.  Only valid button ids can be checked for a mapping.
 */
bool cnButtonMapping_IsMapped(CnButtonMapping* mapping, CnPhysicalButtonId buttonId)
{
	CN_ASSERT_PTR(mapping);
	CN_ASSERT(buttonId != CN_PHYSICAL_BUTTON_ID_INVALID,
		"Looking up logical button for an invalid physical button.");

	return cnButtonMapping_LookUp(mapping, buttonId) != CN_DIGITAL_BUTTON_INVALID;
}

void cnButtonMapping_Map(CnButtonMapping* mapping, CnPhysicalButtonId buttonId, CnDigitalButton* button)
{
	CN_ASSERT_PTR(mapping);
	CN_ASSERT(buttonId != CN_PHYSICAL_BUTTON_ID_INVALID,
		"Cannot map an invalid button id to a digital button");
	CN_ASSERT_PTR(button);

	CN_ASSERT(mapping->numMappings < CN_MAX_BUTTON_MAPPINGS, "Too many buttons mappings created.");

	mapping->physical[mapping->numMappings] = buttonId;
	mapping->logical[mapping->numMappings] = button;

	++mapping->numMappings;
}

CnDigitalButton* cnButtonMapping_LookUp(CnButtonMapping* mapping, CnPhysicalButtonId buttonId)
{
	for (uint32_t i = 0; i < mapping->numMappings; ++i) {
		CN_ASSERT(mapping->physical[i] != CN_PHYSICAL_BUTTON_ID_INVALID,
			"Invalid physical button mapping found.");
		if (mapping->physical[i] == buttonId) {
			return mapping->logical[i];
		}
	}
	return CN_DIGITAL_BUTTON_INVALID;
}
