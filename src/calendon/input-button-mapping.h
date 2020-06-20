#ifndef CN_INPUT_BUTTON_MAPPING_H
#define CN_INPUT_BUTTON_MAPPING_H

#ifdef __cplusplus
extern "C" {
#endif

#include <calendon/cn.h>

#include <calendon/input-digital-button.h>

typedef uint32_t CnPhysicalButtonId;

#define CN_MAX_BUTTON_MAPPINGS 256

/**
 * Indicates a mapping for a digital button does not exist.
 */
#define CN_DIGITAL_BUTTON_INVALID NULL

/**
 * Indicates an invalid physical button mapping in a button mapping.  This allows
 * button mappings to be removed in place.
 */
#define CN_PHYSICAL_BUTTON_ID_INVALID ((uint32_t)(~0))

/**
 * Button mappings convert from digital physical button mappings to
 * digital logical button mappings.
 */
typedef struct {
	CnPhysicalButtonId physical[CN_MAX_BUTTON_MAPPINGS];
	CnDigitalButton* logical[CN_MAX_BUTTON_MAPPINGS];
	uint32_t numMappings;
} CnButtonMapping;

CN_API void cnButtonMapping_Clear(CnButtonMapping* mapping);
CN_API bool cnButtonMapping_IsMapped(CnButtonMapping* mapping, CnPhysicalButtonId buttonId);
CN_API void cnButtonMapping_Map(CnButtonMapping* mapping, CnPhysicalButtonId source, CnDigitalButton* button);
CN_API CnDigitalButton* cnButtonMapping_LookUp(CnButtonMapping* mapping, CnPhysicalButtonId buttonId);

#ifdef __cplusplus
}
#endif

#endif /* CN_INPUT_BUTTON_MAPPING_H */
