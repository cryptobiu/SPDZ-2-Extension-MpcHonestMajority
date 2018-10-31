
#pragma once

#define			GFP_VECTOR		1
#define			GFP_LIMBS		GFP_VECTOR*2
#define			SHR_LIMBS		GFP_LIMBS*2
#define			GFP_BYTES		GFP_LIMBS*sizeof(mp_limb_t)
#define			SHR_BYTES		SHR_LIMBS*sizeof(mp_limb_t)
