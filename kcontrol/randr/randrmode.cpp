#include "randrmode.h"

#ifdef HAS_RANDR_1_2

RandRMode::RandRMode(XRRModeInfo *info)
{
	m_valid = false;
	m_rate = 0;
	
	if (info)
		m_valid = true;
	else
		return;

	m_name = info->name;
	m_id = info->id;

	m_size.setWidth(info->width);
	m_size.setHeight(info->height);
	
	// calculate the refresh rate
	if (info->hTotal && info->vTotal)
		m_rate = ((float) info->dotClock / ((float) info->hTotal * (float) info->vTotal));
	else
		m_rate = 0;

}

RandRMode::~RandRMode()
{
	// nothing to do for now
}

RRMode RandRMode::id() const
{
	if (!m_valid)
		return None;

	return m_id;
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
