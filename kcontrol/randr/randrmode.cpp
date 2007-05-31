#include "randrmode.h"

#ifdef HAS_RANDR_1_2

RandRMode::RandRMode(XRRModeInfo *info)
: m_valid(false)
{
	m_info = info;
	
	if (m_info)
		m_valid = true;
	else
		return;

	m_name = m_info->name;

}

RandRMode::~RandRMode()
{
	// nothing to do for now
	// do NOT delete m_info here, it is handle by RandRScreen
}

QString RandRMode::name() const
{
	return m_name;
}

bool RandRMode::isValid() const
{
	return m_valid;
}

#endif
