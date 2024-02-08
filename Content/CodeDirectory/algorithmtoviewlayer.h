#ifndef ALGORITHMTOVIEWLAYER_H
#define ALGORITHMTOVIEWLAYER_H

#include <QObject>
#include "hcarcsmanager.h"

class AlgorithmToViewLayer :  public QObject
{
    Q_OBJECT
public:
    AlgorithmToViewLayer();
    ~AlgorithmToViewLayer();

    void runHCArcsExample1();
    void runHCArcsExample2();
    void runHCArcsRandExample();
    void runDOPCExample1();
    void runDOPCExample2();

signals:
    void algorithmOutput(const HCArcsOutput &out);
    void dataToDraw(const HCArcsOutput &input_data);

private:
    void init();
    void hcArcsCallback(const HCArcsOutput &out);
    void hcArcsGetInputData(const HCArcsOutput &in);

    HCArcsManager *mp_algorithmManager;
};

#endif // ALGORITHMTOVIEWLAYER_H
