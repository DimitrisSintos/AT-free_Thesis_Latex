#include "hcarcs_object.h"
#include "hcarcs_utils.h"
#include "endpoint.h"

Arc::Arc()
{
    init();
}

Arc::Arc(int head_val, int tail_val)
{
    init();
    mp_head = new Arc_Endpoint(head_val);
    mp_tail = new Arc_Endpoint(tail_val);
}

Arc::Arc(const Arc_Endpoint &head, const Arc_Endpoint &tail)
{
    init();
    mp_head = new Arc_Endpoint(head);
    mp_tail = new Arc_Endpoint(tail);
}

Arc::Arc(Arc &&other)
{
    mp_head = other.mp_head;
    mp_tail = other.mp_tail;
    m_head_order_index = other.m_head_order_index;
    m_tail_order_index = other.m_tail_order_index;
    m_corresponding_cc_int_index = other.m_corresponding_cc_int_index;
    m_inSubsetB = other.m_inSubsetB;

    other.mp_head = nullptr;
    other.mp_tail = nullptr;
    other.m_head_order_index = -1;
    other.m_head_order_index = -1;
    other.m_corresponding_cc_int_index = 1;
    other.m_inSubsetB = false;
}

Arc &Arc::operator=(Arc &&other)
{
    if (this != &other) {

        if (mp_head) delete mp_head;
        if (mp_tail) delete mp_tail;

        mp_head = other.mp_head;
        mp_tail = other.mp_tail;
        m_head_order_index = other.m_head_order_index;
        m_tail_order_index = other.m_tail_order_index;
        m_corresponding_cc_int_index = other.m_corresponding_cc_int_index;
        m_inSubsetB = other.m_inSubsetB;

        other.mp_head = nullptr;
        other.mp_tail = nullptr;
        other.m_head_order_index = -1;
        other.m_head_order_index = -1;
        other.m_corresponding_cc_int_index = -1;
        other.m_inSubsetB = false;
    }

    return *this;
}

Arc::~Arc()
{
    if (mp_head) {
        delete mp_head;
        mp_head = nullptr;
    }
    if (mp_tail) {
        delete mp_tail;
        mp_tail = nullptr;
    }
}

int Arc::calculate_length(int num_of_eps) const
{
    int length = -1;
    int tail_val = mp_tail->value();
    int head_val = mp_head->value();

    if ( tail_val > head_val) {
        length = tail_val - head_val;
    }
    else if ( tail_val < head_val) {
        length = num_of_eps + tail_val - head_val;
    }
    else {
        log_msg("calculate_length(): found arc.head == arc.tail", llevel::ERROR);
    }

    return length;
}

bool Arc::contains_point(int point) const
{
    int a = mp_head->value();
    int b = mp_tail->value();

    if (b > a && (point > a && point < b) ) {
        return true;
    }
    if (a > b && (point > a || point < b)) {
        return true;
    }

    return false;
}

bool Arc::intersects_arc(const Arc &other) const
{
    if (this->contains_point(other.mp_head->value())
            || this->contains_point(other.mp_tail->value())) {
        return true;
    }
    if (other.contains_point(mp_head->value())
            || other.contains_point(mp_tail->value())) {
        return true;
    }

    return false;
}

bool Arc::contains_arc(const Arc &other) const
{
    int b_1 = mp_tail->value();
    int a_2 = other.mp_head->value();
    int b_2 = other.mp_tail->value();

    Arc dummyArc(a_2, b_1);

    if (this->contains_point(a_2) && dummyArc.contains_point(b_2)) {
        return true;
    } else {
        return false;
    }
}

SimpleArc Arc::toSimpleArc()
{
    int head_val = this->head()->value();
    int tail_val = this->tail()->value();
    int order = this->tail_order_index();

    SimpleArc sa(head_val, tail_val, order);

    return sa;
}

void Arc::init()
{
    mp_head = nullptr;
    mp_tail = nullptr;
    m_head_order_index = -1;
    m_tail_order_index = -1;
    m_corresponding_cc_int_index = -1;
    m_inSubsetB = false;
}

// ----

Interval::Interval()
{
    init();
}

Interval::Interval(int left_val, int right_val)
{
    init();
    mp_left = new LeftInterval_Endpoint(left_val);
    mp_right = new RightInterval_Endpoint(right_val);
}

Interval::Interval(const Interval &other)
{
    // deep copy
    mp_left = new LeftInterval_Endpoint(*other.mp_left);
    mp_right = new RightInterval_Endpoint(*other.mp_right);

    m_left_order_index = other.m_left_order_index;
    m_right_order_index = other.m_right_order_index;

    // shallow copy. we only have a single instance of arcs
    mp_originalArcPtr = other.mp_originalArcPtr;

}

Interval::Interval(Interval &&other)
{
    mp_left = other.mp_left;
    mp_right = other.mp_right;
    m_right_order_index = other.m_right_order_index;
    m_left_order_index = other.m_left_order_index;
    mp_originalArcPtr = other.mp_originalArcPtr;

    other.mp_left = nullptr;
    other.mp_right = nullptr;
    other.m_right_order_index = -1;
    other.m_left_order_index = -1;
    other.mp_originalArcPtr = nullptr;
}

Interval &Interval::operator=(Interval &&other)
{
    if (this != &other) {

        if (mp_left) delete mp_left;
        if (mp_right) delete mp_right;

        mp_left = other.mp_left;
        mp_right = other.mp_right;
        m_right_order_index = other.m_right_order_index;
        m_left_order_index = other.m_left_order_index;
        mp_originalArcPtr = other.mp_originalArcPtr;

        other.mp_left = nullptr;
        other.mp_right = nullptr;
        other.m_right_order_index = -1;
        other.m_left_order_index = -1;
        other.mp_originalArcPtr = nullptr;
    }

    return *this;
}

Interval::~Interval()
{
    if (mp_left) {
        delete mp_left;
        mp_left = nullptr;
    }
    if (mp_right) {
        delete mp_right;
        mp_right = nullptr;
    }
    if (mp_originalArcPtr) mp_originalArcPtr = nullptr;
}

bool Interval::intersects_interval(const Interval &other) const
{
    int a_1 = this->mp_left->value();
    int b_1 = this->mp_right->value();
    int a_2 = other.mp_left->value();
    int b_2 = other.mp_right->value();

    if (    ( a_1 < a_2 && a_2 < b_1 ) ||
            ( a_1 < b_2 && b_2 < b_1 ) ||
            ( a_2 < a_1 && b_1 < b_2 )) {
        return true;
    }

    return false;
}

void Interval::init()
{
    mp_left = nullptr;
    mp_right = nullptr;
    m_right_order_index = -1;
    m_left_order_index = -1;
    mp_originalArcPtr = nullptr;
}

// -----------------------------------

const double SimpleArc::ARC_PI = 3.14;

SimpleArc::SimpleArc()
{
    init();
}

SimpleArc::SimpleArc(int head, int tail, int order_index)
{
    init();
    m_head_val = head;
    m_tail_val = tail;
    m_order_index = order_index;
}

SimpleArc::~SimpleArc()
{

}

double SimpleArc::head_rad_val(int num_of_arcs)
{
    return (m_head_val * ARC_PI)/num_of_arcs;
}

double SimpleArc::tail_rad_val(int num_of_arcs)
{
    return (m_tail_val * ARC_PI)/num_of_arcs;
}

double SimpleArc::head_deg_val(int num_of_arcs)
{
    return head_rad_val(num_of_arcs) * 180.0 / ARC_PI;
}

double SimpleArc::tail_deg_val(int num_of_arcs)
{
    return tail_rad_val(num_of_arcs) * 180.0 / ARC_PI;
}

double SimpleArc::startAngle(int num_of_arcs)
{
    return m_head_val * (-180.0/(double)num_of_arcs);
}

double SimpleArc::endAngle(int num_of_arcs)
{
    return m_tail_val * (-180.0/(double)num_of_arcs);
}

double SimpleArc::spanAngle(int num_of_arcs)
{
    if (m_head_val < m_tail_val) {

        return (m_tail_val - m_head_val) * (-180.0/(double)num_of_arcs);
    } else {

        return ((2*num_of_arcs) - m_head_val + m_tail_val) * (-180.0/(double)num_of_arcs);
    }
}

void SimpleArc::init()
{
    m_head_val = -1;
    m_tail_val = -1;
    m_order_index = -1;
}


// -----------------------------------



SimpleInterval::SimpleInterval()
{
    init();
}

SimpleInterval::SimpleInterval(int left, int right, int order_index)
{
    init();
    m_left_val = left;
    m_right_val = right;
    m_order_index = order_index;
}

SimpleInterval::~SimpleInterval()
{

}

void SimpleInterval::init()
{
    m_left_val = -1;
    m_right_val = -1;
    m_order_index = -1;
}
