
#pragma once

#define			VAL_LIMBS		2									//count of 64-bit limbs in a value
#define			VAL_BYTES		VAL_LIMBS*sizeof(mp_limb_t)			//count of bytes in a value

#define			GFP_VALS		1									//count of values contained in a GFP (possibly vectorised)
#define			GFP_LIMBS		GFP_VALS*VAL_LIMBS					//count of 64-bit limbs in a GFP
#define			GFP_BYTES		GFP_LIMBS*sizeof(mp_limb_t)			//count of bytes in a GFP

#define			SHR_GFPS		2									//count of GFP contained in a share
#define			SHR_LIMBS		SHR_GFPS*GFP_LIMBS					//count of 64-bit limbs in a share
#define			SHR_BYTES		SHR_LIMBS*sizeof(mp_limb_t)			//count of bytes in a share
