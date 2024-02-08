#include "endpoint.h"
#include "hcarcs_utils.h"

Arc_Endpoint::Arc_Endpoint()
{
    init();
}

Arc_Endpoint::Arc_Endpoint(int value)
{
    init();
    m_original_value = value;
}

Arc_Endpoint::Arc_Endpoint(const Arc_Endpoint &other)
{
    m_original_value = other.m_original_value;
    m_ascending_order_index = other.m_ascending_order_index;
    m_ArcToI_label = other.m_ArcToI_label;
}

void Arc_Endpoint::init()
{
    m_original_value = -1;
    m_ascending_order_index = -1;
    m_ArcToI_label = -1;
}

// ----


RightInterval_Endpoint::RightInterval_Endpoint()
{
    init();
}

RightInterval_Endpoint::RightInterval_Endpoint(int value)
{
    init();
    m_original_value = value;
}

RightInterval_Endpoint::RightInterval_Endpoint(const RightInterval_Endpoint &other)
{
    // shallow copy

    // better to crash on copying, rather than inside an algorithm
    mp_parent = nullptr;
    mp_Lsuccessor = nullptr;
    mp_Lprevious = nullptr;

    m_original_value = other.m_original_value;
    m_ascending_order_index = other.m_ascending_order_index;
    m_uptreeID = other.m_uptreeID;
}

RightInterval_Endpoint::RightInterval_Endpoint(RightInterval_Endpoint &&other)
{
    mp_parent = other.mp_parent;
    mp_Lsuccessor = other.mp_Lsuccessor;
    mp_Lprevious = other.mp_Lprevious;
    m_original_value = other.m_original_value;
    m_ascending_order_index = other.m_ascending_order_index;
    m_uptreeID = other.m_uptreeID;

    other.mp_parent = nullptr;
    other.mp_Lsuccessor = nullptr;
    other.mp_Lprevious = nullptr;
    other.m_original_value = -1;
    other.m_ascending_order_index = -1;
    other.m_uptreeID = 0;
}

RightInterval_Endpoint &RightInterval_Endpoint::operator=(RightInterval_Endpoint &&other)
{
    if (this != &other) {

        mp_parent = other.mp_parent;
        mp_Lsuccessor = other.mp_Lsuccessor;
        mp_Lprevious = other.mp_Lprevious;
        m_original_value = other.m_original_value;
        m_ascending_order_index = other.m_ascending_order_index;
        m_uptreeID = other.m_uptreeID;

        other.mp_parent = nullptr;
        other.mp_Lsuccessor = nullptr;
        other.mp_Lprevious = nullptr;
        other.m_original_value = -1;
        other.m_ascending_order_index = -1;
        other.m_uptreeID = 0;
    }

    return *this;
}

RightInterval_Endpoint::~RightInterval_Endpoint()
{
    if (mp_parent) mp_parent = nullptr;
    if (mp_Lsuccessor) mp_Lsuccessor = nullptr;
    if (mp_Lprevious) mp_Lprevious = nullptr;
}

void RightInterval_Endpoint::init()
{
    mp_parent = nullptr;
    mp_Lsuccessor = nullptr;
    mp_Lprevious = nullptr;

    m_original_value = -1;
    m_ascending_order_index = -1;
    m_uptreeID = 0;
}

// ----------

LeftInterval_Endpoint::LeftInterval_Endpoint()
{
    init();
}

LeftInterval_Endpoint::LeftInterval_Endpoint(int value)
{
    init();
    m_original_value = value;
}

LeftInterval_Endpoint::LeftInterval_Endpoint(const LeftInterval_Endpoint &other)
{
    // shallow copy -> remember to point the parent

    // better to crash on copying, rather than inside an algorithm
    mp_parent = nullptr;

    m_original_value = other.m_original_value;
    m_ascending_order_index = other.m_ascending_order_index;
    m_uptreeID = other.m_uptreeID;
}

LeftInterval_Endpoint::LeftInterval_Endpoint(LeftInterval_Endpoint &&other)
{
    mp_parent = other.mp_parent;
    m_original_value = other.m_original_value;
    m_ascending_order_index = other.m_ascending_order_index;
    m_uptreeID = other.m_uptreeID;

    other.mp_parent = nullptr;
    other.m_original_value = -1;
    other.m_ascending_order_index = -1;
    other.m_uptreeID = 0;
}

LeftInterval_Endpoint &LeftInterval_Endpoint::operator=(LeftInterval_Endpoint &&other)
{
    if (this != &other) {

        mp_parent = other.mp_parent;
        m_original_value = other.m_original_value;
        m_ascending_order_index = other.m_ascending_order_index;
        m_uptreeID = other.m_uptreeID;

        other.mp_parent = nullptr;
        other.m_original_value = -1;
        other.m_ascending_order_index = -1;
        other.m_uptreeID = 0;
    }

    return *this;
}

LeftInterval_Endpoint::~LeftInterval_Endpoint()
{
    if (mp_parent) mp_parent = nullptr;
}

void LeftInterval_Endpoint::init()
{
    mp_parent = nullptr;

    m_original_value = -1;
    m_ascending_order_index = -1;
    m_uptreeID = 0;
}
