#include "arctoimanager.h"
#include "hcarcs_object.h"
#include "endpoint.h"
#include "hcarcs_utils.h"

ArcToIManager::ArcToIManager()
{
    init();
}

ArcToIManager::ArcToIManager(int num_of_arcs)
{
    init();
    m_num_of_arcs = num_of_arcs;
}

ArcToIManager::~ArcToIManager()
{
    cleanup();
}

bool ArcToIManager::runAlgorithm(Arc *arcsArray, Arc **leftArcsPtrArray, int num_of_arcs, int point_q, int subsetB_indexes[], int subsetB_size, direction dir, Interval *intervalArray_out, Interval **leftIntervalPtrArray_out)
{
    bool ok = false;

    do {

        m_initialized = checkInput(arcsArray, leftArcsPtrArray, num_of_arcs);

        if (!m_initialized) {
            log_msg("ArcToIManager::runAlgorithm(): failed to initialize manager", llevel::ERROR);
            break;
        }
        if (point_q < 0) {
            log_msg("ArcToIManager::runAlgorithm(): point q has negative value", llevel::ERROR);
            break;
        }

        // some "cleanup" on ArcToI specific helper members. In case someone re-uses the arcs

        for (int i = 0; i < m_num_of_arcs; i++) {
            arcsArray[i].head()->setArcToILabel(-1);
            arcsArray[i].tail()->setArcToILabel(-1);
            arcsArray[i].setInsubSetB(false);
        }

        /* Mark "inSubsetB()" arcs. subsetB_size can be 0 in case of empty set */
        for (int i = 0; i < subsetB_size; i ++) {
            arcsArray[subsetB_indexes[i]].setInsubSetB(true);
        }

        /* Mark point q as ArcToI_label = 1 */
        bool found_point_q = false;
        int index_of_point_q = -1;

        for (int i = 0; i < m_num_of_arcs; i++) {

            if (point_q == arcsArray[i].head()->value()) {
                arcsArray[i].head()->setArcToILabel(1);
                index_of_point_q = arcsArray[i].head()->order_index();
                found_point_q = true;
                break;
            }
            if (point_q == arcsArray[i].tail()->value()) {
                arcsArray[i].tail()->setArcToILabel(1);
                index_of_point_q = arcsArray[i].tail()->order_index();
                found_point_q = true;
                break;
            }
        }
        if (!found_point_q) {
            log_msg("ArcToIManager::runAlgorithm(): point q is not included in endpoints", llevel::ERROR);
            break;
        }

        /* ArcToI Start */
        log_msg("ArcToIManager::runAlgorithm(): Printing input", llevel::INFO);
        log_data(arcsArray, m_num_of_arcs, llevel::DEBUG);

        log_msg("point_q value = " + std::to_string(point_q), llevel::DEBUG);


        int max_ep = 2 * m_num_of_arcs;
        log_msg("m_num_of_arcs = " + std::to_string(m_num_of_arcs) + " max_ep = " + std::to_string(max_ep), llevel::DEBUG);

        if (dir == direction::C) {
            log_msg("direction = C", llevel::DEBUG);
        } else {
            log_msg("direction = CC", llevel::DEBUG);
        }

        /* Assign labels to endpoints (affected by direction) */
        log_msg("ArcToIManager::runAlgorithm(): Assign labels to endpoints", llevel::INFO);

        Arc_Endpoint *epPtrArray[max_ep] = {0};

        /* "Reconstruct" endpoint order with ptrs */
        int head_order = -1, tail_order = -1;
        for (int i = 0; i < m_num_of_arcs; i++) {
            head_order = arcsArray[i].head()->order_index();
            tail_order = arcsArray[i].tail()->order_index();

            epPtrArray[head_order] = arcsArray[i].head();
            epPtrArray[tail_order] = arcsArray[i].tail();
        }

        /* Label the endpoints */
        if (dir == direction::C) {

            for (int i = 0; i < max_ep; i++) {
                if (i < index_of_point_q) {
                    epPtrArray[i]->setArcToILabel( max_ep - index_of_point_q + 1 + i);
                } else {
                    epPtrArray[i]->setArcToILabel( i - index_of_point_q + 1);
                }

            }
        }
        if (dir == direction::CC) {

            for (int i = 0; i < max_ep; i++) {
                if (i <= index_of_point_q) {
                    epPtrArray[i]->setArcToILabel( index_of_point_q - i + 1);
                } else {
                    epPtrArray[i]->setArcToILabel( max_ep + index_of_point_q - i + 1);
                }
            }
        }

        /* loop each arc based on the algorithm and create helper endpoints and intervals */
        int left_val = -1, right_val = -1;
        int started_as_one_counter = 0;
        // helper interval array, ordering will be fixed later on through their endpoints
        DInterval *helperIntervalArray = new DInterval[m_num_of_arcs];


        log_msg("ArcToIManager::runAlgorithm(): main loop start", llevel::INFO);

        for (int i = 0; i < m_num_of_arcs; i++) {


            if (dir == direction::C) {

                /* for each arc not in B */
                if (!arcsArray[i].inSubsetB()) {
                    if (arcsArray[i].tail()->ArcToILabel() > arcsArray[i].head()->ArcToILabel()) {
                        // ( l(head(x)), l(tail(x)) )
                        left_val = arcsArray[i].head()->ArcToILabel();
                        right_val = arcsArray[i].tail()->ArcToILabel();

                    } else {
                        // ( l(head(x)), l(tail(x)) + 2N )
                        left_val = arcsArray[i].head()->ArcToILabel();
                        right_val = arcsArray[i].tail()->ArcToILabel() + max_ep;

                    }
                }
                /* for each arc in B */
                if (arcsArray[i].inSubsetB()) {
                    // ( 1 , l(tail(x)) )
                    left_val = 1;
                    right_val = arcsArray[i].tail()->ArcToILabel();
                }
            }

            if (dir == direction::CC) {

                /* for each arc not in B */
                if (!arcsArray[i].inSubsetB()) {

                    if (arcsArray[i].head()->ArcToILabel() > arcsArray[i].tail()->ArcToILabel()) {
                        // ( l(tail(x)), l(head(x)) )
                        left_val = arcsArray[i].tail()->ArcToILabel();
                        right_val = arcsArray[i].head()->ArcToILabel();

                    } else {
                        // ( l(tail(x)), l(head(x)) + 2N )
                        left_val = arcsArray[i].tail()->ArcToILabel();
                        right_val = arcsArray[i].head()->ArcToILabel() + max_ep;
                    }
                }
                /* for each arc in B */
                if (arcsArray[i].inSubsetB()) {
                    // (1 , l(head(x)) )
                    left_val = 1;
                    right_val = arcsArray[i].head()->ArcToILabel();
                }

            }

            // this happens in every iteration, regardless of direction
            // construct dummy interval endpoints and store them to the helper array
            DI_endpoint left_ep;
            if (left_val == 1) {
                left_ep.setStarted_as_one(true);
                started_as_one_counter ++;
            }
            left_ep.setValue(left_val);
            left_ep.setOrder(left_val);
            left_ep.setIs_left(true);
            left_ep.setUn_parent_index(i);

            DI_endpoint right_ep;
            if (right_val == 1) {
                right_ep.setStarted_as_one(true);
                started_as_one_counter ++;
            }
            right_ep.setValue(right_val);
            right_ep.setOrder(right_val);
            right_ep.setIs_right(true);
            right_ep.setUn_parent_index(i);

            // store temp endpoints to helper array
            helperIntervalArray[i] = DInterval(left_ep, right_ep);

        } // ArcToI end

        log_msg("ArcToIManager::runAlgorithm(): main loop end", llevel::INFO);


        /* leaving this as comments as a debug option */
//        for (int i = 0; i < m_num_of_arcs; i++) {
//            DEBUG_OUT << "HI[" << i << "]: val( " << helperIntervalArray[i].left()->value() <<
//                         ", " << helperIntervalArray[i].right()->value() <<
//                         ") order( " << helperIntervalArray[i].left()->order() <<
//                         ", " << helperIntervalArray[i].right()->order() <<
//                         ") p_index( " << helperIntervalArray[i].left()->un_parent_index() <<
//                         ", " << helperIntervalArray[i].right()->un_parent_index() <<
//                         ")"
//                         DEBUG_OUT_END;
//        }

        /* parse helper array and move every endpoint value and order by +N.
         * Only exception are dummy intervals that start with a value = 1.
         * Call these endpoints as one_starters */

        int moved_one_starters_count = 0;
        int temp_orig_val = -1;
        for (int i = 0; i < m_num_of_arcs; i ++) {

            // handle interval.left
            if (helperIntervalArray[i].left()->started_as_one()) {
                helperIntervalArray[i].left()->setValue(1 + moved_one_starters_count);
                helperIntervalArray[i].left()->setOrder(1 + moved_one_starters_count);
                moved_one_starters_count ++;
            } else {
                temp_orig_val = helperIntervalArray[i].left()->value();
                helperIntervalArray[i].left()->setValue(temp_orig_val + m_num_of_arcs);
                helperIntervalArray[i].left()->setOrder(temp_orig_val + m_num_of_arcs);
            }

            // handle interval.right
            if (helperIntervalArray[i].right()->started_as_one()) {
                helperIntervalArray[i].right()->setValue(1 + moved_one_starters_count);
                helperIntervalArray[i].right()->setOrder(1 + moved_one_starters_count);
                moved_one_starters_count ++;
            } else {
                temp_orig_val = helperIntervalArray[i].right()->value();
                helperIntervalArray[i].right()->setValue(temp_orig_val + m_num_of_arcs);
                helperIntervalArray[i].right()->setOrder(temp_orig_val + m_num_of_arcs);
            }
        }


        /* place helper interval endpoints in 1-5N array */
        int fiveN = 5 * m_num_of_arcs;
        DI_endpoint fiveNEndpArray[fiveN];

        for (int i = 0; i < m_num_of_arcs; i++) {
            int right_ep_order = helperIntervalArray[i].right()->order();
            int left_ep_order = helperIntervalArray[i].left()->order();

            fiveNEndpArray[right_ep_order] = *helperIntervalArray[i].right();
            fiveNEndpArray[left_ep_order] = *helperIntervalArray[i].left();
        }

        /* 1 - N space is for case (1, x) */
        /* N+1 - 3N is for case (x,y) */
        /* 3N+1 - 5N is for case (x, y+2N) */


        /* fix order of endpoints. */
        int ordering_of_left_endpoints = 0; // will reach value = N-1
        int ordering_of_right_endpoints = 0; // will reach value = N-1
        int all_endpoint_ordering = 0; // will reach value = 2N-1
        int helperArrayIndex = -1;

        for (int i = 0; i < fiveN; i ++) {

            if (fiveNEndpArray[i].is_left()) {
                helperArrayIndex = fiveNEndpArray[i].un_parent_index();

                helperIntervalArray[helperArrayIndex].setLeft_ordering(ordering_of_left_endpoints);
                helperIntervalArray[helperArrayIndex].left()->setOrder(all_endpoint_ordering); // correct ordering of this endpoint

                ordering_of_left_endpoints ++;
                all_endpoint_ordering ++;

            } else if (fiveNEndpArray[i].is_right()) {
                helperArrayIndex = fiveNEndpArray[i].un_parent_index();

                helperIntervalArray[helperArrayIndex].setRight_ordering(ordering_of_right_endpoints);
                helperIntervalArray[helperArrayIndex].right()->setOrder(all_endpoint_ordering); // correct ordering of this endpoint

                ordering_of_right_endpoints ++;
                all_endpoint_ordering ++;

            } else {
                // we have an empty spot in the array, skip it
                continue;
            }
        }

        /* create actual intervals */

        for (int i = 0; i < m_num_of_arcs; i++) {
            int left_val = helperIntervalArray[i].left()->value();
            int left_ep_order = helperIntervalArray[i].left()->order();
            int left_ordering = helperIntervalArray[i].left_ordering();

            int right_val = helperIntervalArray[i].right()->value();
            int right_ep_order = helperIntervalArray[i].right()->order();
            int right_ordering = helperIntervalArray[i].right_ordering();


            // intervalArray_out[] : holds the intervals in order based on their right endpoints
            intervalArray_out[right_ordering] = Interval(left_val, right_val);
            intervalArray_out[right_ordering].left()->setParent(&intervalArray_out[right_ordering]);
            intervalArray_out[right_ordering].left()->set_order(left_ep_order);
            intervalArray_out[right_ordering].right()->setParent(&intervalArray_out[right_ordering]);
            intervalArray_out[right_ordering].right()->set_order(right_ep_order);
            intervalArray_out[right_ordering].setMp_originalArc(&arcsArray[i]); //reminder that helperIntervalArray holds the same "order" with arcsArray
            intervalArray_out[right_ordering].setRight_order_index(right_ordering);
            intervalArray_out[right_ordering].setLeft_order_index(left_ordering);

            // leftIntervalPtrArray_out[] : holds pointers the intervals in order based on their left endpoints
            leftIntervalPtrArray_out[left_ordering] = &intervalArray_out[right_ordering];
        }


        if (helperIntervalArray) {
            delete [] helperIntervalArray;
            helperIntervalArray = nullptr;
        }

        log_msg("ArcToIManager::runAlgorithm(): printing output intervals", llevel::INFO);
        log_data(intervalArray_out, m_num_of_arcs, llevel::DEBUG);
        log_data(leftIntervalPtrArray_out, m_num_of_arcs, llevel::DEBUG);


        log_msg("ArcToIManager::runAlgorithm(): end", llevel::INFO);
        ok = true;
    } while(0);

    return ok;
}

void ArcToIManager::resetManager()
{
    cleanup();
}

void ArcToIManager::init()
{
    m_num_of_arcs = 0;
    m_initialized = false;
}

void ArcToIManager::cleanup()
{
    // empty. we would call the destructor here, but we have no member-allocated memory to free
}

bool ArcToIManager::checkInput(Arc *arcsArray, Arc **leftArcsPtrArray, int num_of_arcs)
{
    bool ok = false;

    do {

        log_msg("ArcToIManager::checkInput(): start", llevel::INFO);

        if (arcsArray == nullptr) {
            log_msg("ArcToIManager::checkInput(): null array pointer in this function input", llevel::ERROR);
            break;
        }
        if (leftArcsPtrArray == nullptr) {
            log_msg("ArcToIManager::checkInput(): null array pointer in this function input", llevel::ERROR);
            break;
        }
        if (num_of_arcs != m_num_of_arcs) {
            log_msg("ArcToIManager::checkInput(): size != m_num_of_arcs", llevel::ERROR);
            break;
        }

        /* check for endpoint value and order.
         * Value must be within [1, 2N].
         * Order must be withing [0, 2N-1]. */

        bool ep_within_bounds_ok = true;
        int max_ep = 2*num_of_arcs;

        for (int i = 0; i < m_num_of_arcs; i ++) {

            // value check
            if (arcsArray[i].tail()->value() < 1 ||
                    arcsArray[i].head()->value() < 1 ) {
                ep_within_bounds_ok = false;
                break;
            }
            if (arcsArray[i].tail()->value() > max_ep ||
                    arcsArray[i].head()->value() > max_ep ) {
                ep_within_bounds_ok = false;
                break;
            }

            // order check
            if (arcsArray[i].tail()->order_index() < 0 ||
                    arcsArray[i].head()->order_index() < 0 ) {
                ep_within_bounds_ok = false;
                break;
            }
            if (arcsArray[i].tail()->order_index() > max_ep -1  ||
                    arcsArray[i].head()->order_index() > max_ep -1 ) {
                ep_within_bounds_ok = false;
                break;
            }
        }

        if (!ep_within_bounds_ok) {
            log_msg("ArcToIManager::checkInput(): input has arc with endpoint out of bounds", llevel::ERROR);
            break;
        }

        /* check ep ordering */

        bool ep_ordering_ok = true;
        // recreate ep ordering from arcs

        Arc_Endpoint *epPtrArray[max_ep] = {0}; // max_ep = 2N

        /* "Reconstruct" endpoint order with ptrs */
        int head_order = -1, tail_order = -1;
        for (int i = 0; i < m_num_of_arcs; i++) {
            head_order = arcsArray[i].head()->order_index();
            tail_order = arcsArray[i].tail()->order_index();

            epPtrArray[head_order] = arcsArray[i].head();
            epPtrArray[tail_order] = arcsArray[i].tail();
        }

        for (int i = 0; i < max_ep; i ++) {

            if (epPtrArray[i]->order_index() != i) {
                ep_ordering_ok = false;
                break;
            }
        }

        if (!ep_ordering_ok) {
            log_msg("ArcToIManager::checkInput(): input does not have correct endpoint ordering", llevel::ERROR);
            break;
        }



        /* check if input is in ascending tail order*/
        int prev_tail_val = -1;
        bool ascending_tail_ordering_ok = true;

        prev_tail_val = arcsArray[0].tail()->value();
        for (int i = 1; i < m_num_of_arcs; i++) {

            if (prev_tail_val > arcsArray[i].tail()->value()) {
                log_msg("prev_tail_val > arcsArray[i].tail()->value()", llevel::ERROR);
                ascending_tail_ordering_ok = false;
                break;
            }
            if (arcsArray[i].tail_order_index() != i){
                log_msg("arcsArray[i].tail_order_index() != i", llevel::ERROR);
                ascending_tail_ordering_ok = false;
                break;
            }
            prev_tail_val = arcsArray[i].tail()->value();
        }

        if ( arcsArray[0].tail_order_index() != 0) {
            log_msg("arcsArray[0].tail_order_index() != 0" , llevel::ERROR);
            ascending_tail_ordering_ok = false;
        }

        if (!ascending_tail_ordering_ok) {
            log_msg("ArcToIManager::checkInput(): input is not ordered based on tails (ascending) ", llevel::ERROR);
            break;
        }

        /* check if input is in ascending head order*/
        int prev_head_val = -1;
        bool ascending_head_ordering_ok = true;

        prev_head_val = leftArcsPtrArray[0]->head()->value();
        for (int i = 1; i < m_num_of_arcs; i++) {

            if (prev_head_val > leftArcsPtrArray[i]->head()->value()) {
                log_msg("prev_head_val > leftArcsPtrArray[i]->head()->value()", llevel::ERROR);
                ascending_head_ordering_ok = false;
                break;
            }
            if (leftArcsPtrArray[i]->head_order_index() != i){
                log_msg("leftArcsPtrArray[i]->head_order_index() != i", llevel::ERROR);
                ascending_head_ordering_ok = false;
                break;
            }
            prev_head_val = leftArcsPtrArray[i]->head()->value();
        }
        if (leftArcsPtrArray[0]->head_order_index() != 0) {
            log_msg("leftArcsPtrArray[0]->head_order_index() != 0", llevel::ERROR);
            ascending_head_ordering_ok = false;
        }

        if (!ascending_head_ordering_ok) {
            log_msg("ArcToIManager::checkInput(): input is not ordered based on heads (ascending) " , llevel::ERROR);
            break;
        }

        log_msg("ArcToIManager::checkInput(): end", llevel::INFO);
        ok = true;
    } while(0);

    return ok;
}

// ------------------------------------------------------------

DInterval::DInterval()
{
    init();
}

DInterval::DInterval(const DI_endpoint &left, const DI_endpoint &right)
{
    init();
    mp_left = new DI_endpoint(left);
    mp_right = new DI_endpoint(right);
}

DInterval::DInterval(DInterval &&other)
{
    mp_left = other.mp_left;
    mp_right = other.mp_right;
    m_left_ordering = other.m_left_ordering;
    m_right_ordering = other.m_right_ordering;

    other.mp_left = nullptr;
    other.mp_right = nullptr;
    other.m_left_ordering = -1;
    other.m_right_ordering = -1;
}

DInterval &DInterval::operator=(DInterval &&other)
{
    if (this != &other) {

        if (mp_left) delete mp_left;
        if (mp_right) delete mp_right;

        mp_left = other.mp_left;
        mp_right = other.mp_right;
        m_left_ordering = other.m_left_ordering;
        m_right_ordering = other.m_right_ordering;

        other.mp_left = nullptr;
        other.mp_right = nullptr;
        other.m_left_ordering = -1;
        other.m_right_ordering = -1;
    }

    return *this;
}

DInterval::~DInterval()
{
    if (mp_left) {
        delete mp_left;
        mp_left = nullptr;
    }
    if (mp_right) {
        delete mp_right;
        mp_right = nullptr;
    }
}

void DInterval::init()
{
    mp_left = nullptr;
    mp_right = nullptr;
    m_left_ordering = -1;
    m_right_ordering = -1;
}
