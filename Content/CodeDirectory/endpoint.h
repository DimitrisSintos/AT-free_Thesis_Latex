#ifndef ENDPOINT_H
#define ENDPOINT_H

/* We are dealing with raw pointers, sorting and re-sorting arrays and storing indexes.
 * That's why we want to minimize data copying */

class Endpoint
{
public:
    Endpoint() {};
};

class Arc;

class Arc_Endpoint
{
public:
    Arc_Endpoint();
    Arc_Endpoint(int value);
    Arc_Endpoint(const Arc_Endpoint &other);

    int value() const {return m_original_value;}
    int order_index() const {return m_ascending_order_index;}
    int ArcToILabel() const {return m_ArcToI_label;}
    void set_order(int order) {m_ascending_order_index = order;}
    void setArcToILabel(int label) {m_ArcToI_label = label;}

private:
    void init();

    int m_original_value; // value must be within [1, 2N]
    int m_ascending_order_index; // endpoint ordering in [0, ..., 2N-1]
    int m_ArcToI_label; // used by algorithm ArcToI
};

class Interval;
class LeftInterval_Endpoint;

class RightInterval_Endpoint
{
    friend class DOPCManager;
public:
    RightInterval_Endpoint();
    RightInterval_Endpoint(int value);
    // copying should be done very carefully. Only used by algorithm HCArcs
    RightInterval_Endpoint(const RightInterval_Endpoint &other);
    RightInterval_Endpoint& operator=(const RightInterval_Endpoint &other) = delete;
    // move sematics
    RightInterval_Endpoint(RightInterval_Endpoint &&other);
    RightInterval_Endpoint& operator=(RightInterval_Endpoint &&other);
    ~RightInterval_Endpoint();

    int value() const {return m_original_value;}
    int order_index() const {return m_ascending_order_index;}
    int uptreeID() const {return m_uptreeID;}
    void set_order(int order) {m_ascending_order_index = order;}
    void setUptreeID (int id) {m_uptreeID = id;}
    Interval *parent() {return mp_parent;}
    void setParent(Interval *parent_ptr) {mp_parent = parent_ptr;}

private:
    void init();

    Interval *mp_parent;
    // used by algorithm DOPC
    RightInterval_Endpoint *mp_Lsuccessor;
    RightInterval_Endpoint *mp_Lprevious;

    int m_original_value; // no upper limit on value range
    int m_ascending_order_index; // endpoint ordering in [0, ..., 2N-1]
    int m_uptreeID; // used by algorithm DOPC, it can get negative values as well
};

class LeftInterval_Endpoint
{
public:
    LeftInterval_Endpoint();
    LeftInterval_Endpoint(int value);
    // copying should be done very carefully. Only used by algorithm HCArcs
    LeftInterval_Endpoint(const LeftInterval_Endpoint &other);
    LeftInterval_Endpoint& operator=(const LeftInterval_Endpoint &other) = delete;
    // move sematics
    LeftInterval_Endpoint(LeftInterval_Endpoint &&other);
    LeftInterval_Endpoint& operator=(LeftInterval_Endpoint &&other);
    ~LeftInterval_Endpoint();

    int value() const {return m_original_value;}
    int order_index() const {return m_ascending_order_index;}
    int uptreeID() const {return m_uptreeID;}
    void set_order(int order) {m_ascending_order_index = order;}
    void setUptreeID (int id) {m_uptreeID = id;}
    Interval *parent() {return mp_parent;}
    void setParent(Interval *parent_ptr) {mp_parent = parent_ptr;}

private:
    void init();

    Interval *mp_parent;

    int m_original_value; // no upper limit on value range
    int m_ascending_order_index; // endpoint ordering in [0, ..., 2N-1]
    int m_uptreeID; // used by algorithm DOPC, it can get negative values as well
};


#endif // ENDPOINT_H
