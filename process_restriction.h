#pragma once

struct ProcessRestriction
{
	unsigned _vm_size_max;
	
	explicit ProcessRestriction(unsigned vm_size_max)
		: _vm_size_max(vm_size_max)
	{		
	}
};
