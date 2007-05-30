#include "randrmode.h"

#ifdef HAS_RANDR_1_2

RandRMode::RandRMode(XRRModeInfo *info)
: m_valid(false)
{
	m_info = info;
	
	if (m_info)
		m_valid = true;

}

RandRMode::~RandRMode()
{
	// nothing to do for now
	// do NOT delete m_info here, it is handle by RandRScreen
}

bool RandRMode::isValid() const
{
	return m_valid;
}

#endif
