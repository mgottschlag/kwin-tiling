
#include "clocknumber.h"

void Number::operator--()
{
    if (m_data == '0') {
        m_data = '9';
    } else {
        m_data--;
    }
}

void Number::operator++()
{
    if (m_data == '9') {
        m_data = '0';
    } else {
        m_data++;
    }
}

