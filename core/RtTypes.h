#ifndef _RTTYPES_H_
#define _RTTYPES_H_

#include <QVector>
#include <QMetaType>

typedef QVector<int> RtIntVector;
typedef QVector<unsigned int> RtUintVector;
typedef QVector<double> RtDoubleVector;

Q_DECLARE_METATYPE(RtIntVector)
Q_DECLARE_METATYPE(RtUintVector)
Q_DECLARE_METATYPE(RtDoubleVector)

class QScriptEngine;

int registerVectorTypes(QScriptEngine* eng);

#endif

