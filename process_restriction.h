#pragma once

/*!
 \brief Combination of restrictions for any watched process.
 */
struct ProcessRestriction
{
	unsigned _vm_size_max;
	
	explicit ProcessRestriction(unsigned vm_size_max)
		: _vm_size_max(vm_size_max)
	{		
	}
};
