#ifndef HCARCS_OBJECT_H
#define HCARCS_OBJECT_H

#include "hcarcs_utils.h"

/* We are dealing with raw pointers, sorting and re-sorting arrays and storing indexes.
 * That's why we want to minimize data copying */

class Arc_Endpoint;
class SimpleArc;

class Arc
{
public:
    Arc();
    Arc(int head_val, int tail_val);
    Arc(const Arc_Endpoint &head, const Arc_Endpoint &tail);
    // no copying allowed, use std::move() if must
    Arc(const Arc &other) = delete;
    Arc& operator=(const Arc &other) = delete;
    // move sematics
    Arc(Arc &&other);
    Arc& operator=(Arc &&other);
    ~Arc();

    Arc_Endpoint *head() {return mp_head;}
    Arc_Endpoint *tail() {return mp_tail;}
    int calculate_length(int num_of_eps) const; // the length of the arc. Used to find the arc with the minimum length
    bool contains_point(int point) const;
    bool intersects_arc(const Arc &other) const;
    bool contains_arc(const Arc &other) const;
    int head_order_index() const {return m_head_order_index;}
    int tail_order_index() const {return m_tail_order_index;}
    int corr_cc_int_index() const {return m_corresponding_cc_int_index;}
    void setHead_order_index(int head_index) {m_head_order_index = head_index;}
    void setTail_order_index(int tail_index) {m_tail_order_index = tail_index;}
    void setCorr_cc_int_index(int cc_interval_index) {m_corresponding_cc_int_index = cc_interval_index;}
    bool inSubsetB() const {return m_inSubsetB;}
    void setInsubSetB(bool in) {m_inSubsetB = in;}

    SimpleArc toSimpleArc(); // convert Arc to SimpleArc, for view

private:
    void init();

    Arc_Endpoint *mp_head;
    Arc_Endpoint *mp_tail;
    int m_head_order_index; //index of arc in HCArcs, mp_arcsPtrArray
    int m_tail_order_index; //index of arc in HCArcs, mp_arcsArray
    bool m_inSubsetB; // used by algorithm ArcToI
    int m_corresponding_cc_int_index; // used by algorithm HCArcs. An O(1) way to store the corresponding interval created by ArcToI CC.
};

class LeftInterval_Endpoint;
class RightInterval_Endpoint;

class Interval
{
public:
    Interval();
    Interval(int left_val, int right_val);
    // copying should be done very carefully. Only used by algorithm HCArcs
    Interval(const Interval &other);
    Interval& operator=(const Interval &other) = delete;
    // move sematics
    Interval(Interval &&other);
    Interval& operator=(Interval &&other);
    ~Interval();

    LeftInterval_Endpoint* left() {return mp_left;}
    RightInterval_Endpoint* right() {return mp_right;}
    bool intersects_interval(const Interval &other) const;
    int left_order_index() const {return m_left_order_index;}
    void setLeft_order_index(int left_order_index) {m_left_order_index = left_order_index;}
    int right_order_index() const {return m_right_order_index;}
    void setRight_order_index(int right_order_index) {m_right_order_index = right_order_index;}
    Arc *getMp_originalArc() const {return mp_originalArcPtr;}
    void setMp_originalArc(Arc *arc_ptr) {mp_originalArcPtr = arc_ptr;}


private:
    void init();

    LeftInterval_Endpoint *mp_left;
    RightInterval_Endpoint *mp_right;
    int m_right_order_index; // index of interval based on its right endpoint in [0, ..., N-1]
    int m_left_order_index; // index of interval based on its left endpoint in [0, ..., N-1]
    Arc *mp_originalArcPtr; // pointer to an arc. Algorithm ArcToI makes this connection
};


/* Simple data classes to carry over to output/GUI.
 * We don't care so much about preformance there, just values. No raw pointers either.
 * Copying them is fine. GUI will just read the values, not change them */

class SimpleArc
{
public:

    SimpleArc();
    SimpleArc(int head, int tail, int order_index);
    ~SimpleArc();

    int head_val() {return m_head_val;}
    int tail_val() {return m_tail_val;}
    int getOrderIndex() {return m_order_index;}

    // functions to represent/draw the arc outside HCArcs-related algorithms
    double head_rad_val(int num_of_arcs);
    double tail_rad_val(int num_of_arcs);
    double head_deg_val(int num_of_arcs);
    double tail_deg_val(int num_of_arcs);
    double startAngle(int num_of_arcs); // returns start angle in degrees, for drawing the arc with Qt
    double endAngle(int num_of_arcs); // returns end angle in degrees, for drawing endpoints
    double spanAngle(int num_of_arcs); // returns span angle in degrees, for drawing the arc with Qt

    static const double ARC_PI; // const for PI = 3.14

private:
    void init();

    int m_head_val;
    int m_tail_val;
    int m_order_index;
};

class SimpleInterval
{
public:

    SimpleInterval();
    SimpleInterval(int left, int right, int order_index);
    ~SimpleInterval();

    int left_val() {return m_left_val;}
    int right_val() {return m_right_val;}
    int getOrderIndex() {return m_order_index;}


private:
    void init();

    int m_left_val;
    int m_right_val;
    int m_order_index;
};

#endif // HCARCS_OBJECT_H
