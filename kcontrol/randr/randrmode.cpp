#include "randrmode.h"

#ifdef HAS_RANDR_1_2

RandRMode::RandRMode(XRRModeInfo *info)
: m_valid(false)
{
	m_info = info;
	m_rate = 0;
	
	if (m_info)
		m_valid = true;
	else
		return;

	m_name = m_info->name;

	m_size.setWidth(m_info->width);
	m_size.setHeight(m_info->height);
	
	// calculate the refresh rate
	if (info->hTotal && info->vTotal)
		m_rate = ((float) info->dotClock / ((float) info->hTotal * (float) info->vTotal));
	else
		m_rate = 0;

}

RandRMode::~RandRMode()
{
	// nothing to do for now
	// do NOT delete m_info here, it is handle by RandRScreen
}

RRMode RandRMode::id() const
{
	if (!m_valid)
		return None;

	return m_info->id;
}

QString RandRMode::name() const
{
	return m_name;
}

QSize RandRMode::size() const
{
	return m_size;
}

float RandRMode::refreshRate() const
{
	return m_rate;
}

bool RandRMode::isValid() const
{
	return m_valid;
}

#endif
