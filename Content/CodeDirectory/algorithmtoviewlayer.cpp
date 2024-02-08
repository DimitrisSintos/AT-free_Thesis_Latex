#include "algorithmtoviewlayer.h"

AlgorithmToViewLayer::AlgorithmToViewLayer()
{
    init();
    mp_algorithmManager = new HCArcsManager;
    mp_algorithmManager->setOutputFunc(std::bind(&AlgorithmToViewLayer::hcArcsCallback, this, std::placeholders::_1));
    mp_algorithmManager->setInputDataToViewFunc(std::bind(&AlgorithmToViewLayer::hcArcsGetInputData, this, std::placeholders::_1));
    mp_algorithmManager->setToView(true);
}

AlgorithmToViewLayer::~AlgorithmToViewLayer()
{
    if (mp_algorithmManager) {
        delete mp_algorithmManager;
        mp_algorithmManager = nullptr;
    }
}

void AlgorithmToViewLayer::runHCArcsExample1()
{
    if (!mp_algorithmManager) {
        return;
    }
    mp_algorithmManager->resetManager();
    mp_algorithmManager->testHCArcsExample1();
}

void AlgorithmToViewLayer::runHCArcsExample2()
{
    if (!mp_algorithmManager) {
        return;
    }
    mp_algorithmManager->resetManager();
    mp_algorithmManager->testHCArcsExample2();
}

void AlgorithmToViewLayer::runHCArcsRandExample()
{
    if (!mp_algorithmManager) {
        return;
    }
    mp_algorithmManager->resetManager();

    int N = 10;
    mp_algorithmManager->setNumOfArcs(N);
    mp_algorithmManager->testHCArcsRandExample(N);
}

void AlgorithmToViewLayer::runDOPCExample1()
{
    if (!mp_algorithmManager) {
        return;
    }
    mp_algorithmManager->resetManager();
    mp_algorithmManager->testDOPC_example1();
}

void AlgorithmToViewLayer::runDOPCExample2()
{
    if (!mp_algorithmManager) {
        return;
    }
    mp_algorithmManager->resetManager();
    mp_algorithmManager->testDOPC_example2();
}

void AlgorithmToViewLayer::init()
{
    mp_algorithmManager = nullptr;
}

void AlgorithmToViewLayer::hcArcsCallback(const HCArcsOutput &out)
{
    emit algorithmOutput(out);
}

void AlgorithmToViewLayer::hcArcsGetInputData(const HCArcsOutput &in)
{
    emit dataToDraw(in);
}
