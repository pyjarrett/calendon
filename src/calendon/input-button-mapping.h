#ifndef CN_INPUT_BUTTON_MAPPING_H
#define CN_INPUT_BUTTON_MAPPING_H

#include <calendon/cn.h>

#include <calendon/input-digital-button.h>

typedef uint32_t CnPhysicalButtonId;

#define CN_MAX_BUTTON_MAPPINGS 256
#define CN_INVALID_DIGITAL_BUTTON NULL
#define CN_INVALID_PHYSICAL_BUTTON (~0)

/**
 * Button mappings convert from from digital physical button mappings to
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
CN_API CnDigitalButton* cnButtonMapping_LookUp(CnButtonMapping* mapping, CnPhysicalButtonId source);

#endif /* CN_INPUT_BUTTON_MAPPING_H */
