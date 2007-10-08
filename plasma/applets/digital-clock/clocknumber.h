
#ifndef CLOCKNUMBER_H
#define CLOCKNUMBER_H

#include <QChar>

class Number {
    public:
        Number(QChar value) { m_data = value.toAscii(); }
        void operator--();
        void operator++();
        void operator=(QChar value) { m_data = value.toAscii(); }
        bool operator==(char value) { return m_data == value; }
        operator char() { return m_data; }
    private:
        char m_data;
};

#endif

