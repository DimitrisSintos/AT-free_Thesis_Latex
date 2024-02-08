#ifndef ArcToIMANAGER_H
#define ArcToIMANAGER_H

#include "hcarcs_utils.h"

class Arc;
class Interval;

class ArcToIManager
{
public:
    ArcToIManager();
    ArcToIManager(int num_of_arcs);
    ~ArcToIManager();

    /* Input: arcs sorted based on their tails.
     * arcs sorted based on their heads.
     * number of arcs.
     * the value of an endpoint point q.
     * subsetB.
     * the direction, either Clockwise or Counterclockwise.
     * Output: intervals sorted based on their right endpoint.
     * intervals sorted based on their left endpoint */
    bool runAlgorithm(Arc *arcsArray, Arc **leftArcsPtrArray, int num_of_arcs,
                      int point_q, int subsetB_indexes[], int subsetB_size, direction dir,
                      Interval *intervalArray_out, Interval **leftIntervalPtrArray_out);

    void setNumOfArcs(int n) {m_num_of_arcs = n;}
    void resetManager();

private:
    void init();
    void cleanup();
    bool checkInput(Arc *arcsArray, Arc **leftArcsPtrArray, int num_of_arcs);

    int m_num_of_arcs;
    bool m_initialized;
};

class DI_endpoint
{
    /* helper class for ArcToI algorithm.
     * Simulates an interval endpoint: DummyInterval_endpoint */
public:
    DI_endpoint() {init();}

    int value() {return m_value;}
    int order() {return m_order;}
    int un_parent_index() {return m_unordered_parent_index;}
    bool is_left() {return m_is_left;}
    bool is_right() {return m_is_right;}
    bool started_as_one() {return m_started_as_one;}

    void setValue(int val) {m_value = val;}
    void setOrder(int order) {m_order = order;}
    void setUn_parent_index(int index) {m_unordered_parent_index = index;}
    void setIs_left(bool b) {m_is_left = b;}
    void setIs_right(bool b) {m_is_right = b;}
    void setStarted_as_one(bool b) {m_started_as_one = b;}

private:
    void init() {
        m_value = -1;
        m_order = -1;
        m_unordered_parent_index = -1;
        m_is_left = false;
        m_is_right = false;
        m_started_as_one = false;
    }

    int m_value;
    // endpoint order in [0, ... , 2N-1]
    int m_order;
    // index to an unordered interval array, instead of pointer
    int m_unordered_parent_index;
    // type of endpoint, either left or right
    bool m_is_left;
    bool m_is_right;
    // algorithm needs to know which endpoint started as value = 1 to create distinct interval endpoints later on
    bool m_started_as_one;
};

class DInterval
{
    /* helper class for ArcToI algorithm.
     * Simulates an interval : DummyInterval */
public:
    DInterval();
    DInterval(const DI_endpoint &left, const DI_endpoint &right);
    DInterval(const DInterval &other) = delete;
    DInterval& operator=(const DInterval &other) = delete;
    DInterval(DInterval &&other);
    DInterval& operator=(DInterval &&other);
    ~DInterval();

    DI_endpoint* left() {return mp_left;}
    DI_endpoint* right() {return mp_right;}
    int left_ordering() {return m_left_ordering;}
    int right_ordering() {return m_right_ordering;}

    void setLeft_ordering(int order) {m_left_ordering = order;}
    void setRight_ordering(int order) {m_right_ordering = order;}

private:
    void init();

    DI_endpoint *mp_left;
    DI_endpoint *mp_right;

    /* Interval ordering in [0 , ..., N-1].
     * Intervals are either sorted by their right or their left endpoints */
    int m_right_ordering;
    int m_left_ordering;
};

#endif // ArcToIMANAGER_H
