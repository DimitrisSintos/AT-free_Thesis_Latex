#include "hcarcsmanager.h"
#include "endpoint.h"
#include "hcarcs_object.h"
#include "arctoimanager.h"
#include "dopcmanager.h"
#include "hcintervalsmanager.h"
// for random
#include <cstdlib>
#include <ctime>
#include <cmath>
//
#include "hcarcs_utils.h"

HCArcsManager::HCArcsManager()
{
    init();
}

HCArcsManager::HCArcsManager(int num_of_arcs)
{
    init();
    m_num_of_arcs = num_of_arcs;
}

HCArcsManager::~HCArcsManager()
{
    cleanup();
}

bool HCArcsManager::runAlgorithm(Arc *arcsArray, Arc **leftArcsPtrArray, int num_of_arcs)
{
    bool ok = false;

    // declarations
    Interval *intervalArray_c = nullptr;
    Interval **intervalPtrArray_c = nullptr;
    int *set_Ba_indexes = nullptr;
    std::list<std::list<int>> *pathCover_P = nullptr;
    int *set_Bp_tail_of_up_indexes = nullptr;
    Interval *intervalArray_cc = nullptr;
    Interval **intervalPtrArray_cc = nullptr;
    std::list<std::list<int>> *pathCover_Q = nullptr;
    int *set_Bp_head_of_w_indexes = nullptr;
    Interval *I_s_Array = nullptr;
    Interval **I_s_ptrArray = nullptr;
    std::list<std::list<int>> *pathCover_Q_s = nullptr;
    std::list<int> *intervalsCycle = nullptr;

    do {

        log_asteriskLine();
        log_asteriskLine();
        log_msg("HCArcsManager::runAlgorithm(): start", llevel::INFO);

        // check input
        m_initialized = checkInput(arcsArray, leftArcsPtrArray, num_of_arcs);

        if (!m_initialized) {
            log_msg("HCArcsManager::runAlgorithm(): input failed to pass checks", llevel::ERROR);
            break;
        }

        // move input to member variables
        bool init_ok = false;
        init_ok = initializeMembers(arcsArray, leftArcsPtrArray, num_of_arcs);

        if (!init_ok) {
            log_msg("HCArcsManager::runAlgorithm(): failed to initialize manager's members", llevel::ERROR);
            break;
        }

        // pass data to view
        if (m_inputDataToViewFunc && m_toView) {

            std::list<int> empty_list;
            HCArcsOutput input_data(outputCase::ONLY_ARCS,
                                    nullptr, 0,
                                    mp_arcsArray, m_num_of_arcs,
                                    empty_list, empty_list,
                                    empty_list);

            m_inputDataToViewFunc(input_data);
        }

        /* checks and init done, starting HC-Arcs */
        log_msg("HCArcsManager::runAlgorithm(): printing input sized " + std::to_string(m_num_of_arcs), llevel::INFO);
        log_data(mp_arcsArray, m_num_of_arcs, llevel::DEBUG);
        log_data(mp_arcsPtrArray, m_num_of_arcs, llevel::DEBUG);

        /* step 1: Pick an arc {arc_m} of F {arcsArray} that does not contain any other arc */
        log_msg("/* step 1: Pick an arc {arc_m} of F {arcsArray} that does not contain any other arc */", llevel::INFO);
        int index_arc_m = findIndexOfArcContainingNoOther(mp_arcsArray, m_num_of_arcs);
        log_msg("found arc_m with index = " +std::to_string(index_arc_m), llevel::DEBUG);

        /* step 2-3:  compute F_c = F_c(B_a(arc_m), head(arc_m)) and consturct interval model I_c from ArcToI */
        log_msg("/* step 2-3:  compute F_c = F_c(B_a(arc_m), head(arc_m)) and consturct interval model I_c from ArcToI */", llevel::INFO);

        // calculate B_a(arc_m)
        int Ba_size;
        compute_set_Ba(mp_arcsArray[index_arc_m], set_Ba_indexes, Ba_size);

        // initialize data sets here
        intervalArray_c = new Interval[m_num_of_arcs];
        intervalPtrArray_c = new Interval*[m_num_of_arcs];

        // call ArcToI with B_a(arc_m), head(arc_m), clockwise
        log_dashedLine(); // ---

        bool ArcToI_ok = false;
        ArcToIManager arcToIManager(m_num_of_arcs);
        ArcToI_ok = arcToIManager.runAlgorithm(mp_arcsArray, mp_arcsPtrArray, m_num_of_arcs,
                                                mp_arcsArray[index_arc_m].head()->value(),
                                                set_Ba_indexes, Ba_size, direction::C,
                                                intervalArray_c, intervalPtrArray_c);

        if (!ArcToI_ok) {
            log_msg("HCArcsManager::runAlgorithm(): ArcToI failed to run with clockwise direction", llevel::ERROR);
            break;
        }
        log_dashedLine(); // ---

        /* step 4: call procedure DOPC to obtain path P from I_c */
        log_msg("/* step 4: call procedure DOPC to obtain path P from I_c */", llevel::INFO);

        log_dashedLine(); // ---
        bool dopc_for_P_ok = false;
        DOPCManager dopcManager(m_num_of_arcs);

        pathCover_P = new std::list<std::list<int>>();

        dopc_for_P_ok = dopcManager.runAlgorithm(intervalArray_c, intervalPtrArray_c, m_num_of_arcs, *pathCover_P);

        if (!dopc_for_P_ok) {
            log_msg("HCArcsManager::runAlgorithm(): DOPC failed to run", llevel::ERROR);
            break;
        }
        log_dashedLine(); // ---

        /* step 5: construct path P from pathCover_P, store C(P) as well.
         * Working on intervals, on Clockwise direction */
        log_msg("/* step 5: construct path P from pathCover_P, store C(P) as well. Working with intervals, on Clockwise direction */", llevel::INFO);

        std::list<int> path_P = pathCover_P->front(); // path_P is the first path of DOPC's path cover
        std::list<int> corresp_arc_indexes_toPathP; // the corresponding arc indexes. Needed to present output to view.

        for (std::list<int>::iterator it = path_P.begin(); it != path_P.end(); it++) {
            int corresponding_arc_index = intervalArray_c[*it].getMp_originalArc()->tail_order_index();
            corresp_arc_indexes_toPathP.push_back(corresponding_arc_index);
        }

        std::list<int> setC_of_path_P; // set C(P).
        std::list<std::list<int>> components_of_path_P;
        std::list<int> current_component_of_path_P;
        bool is_new_component_of_path_P = false;

        int current_end_P_index = path_P.back(); // end(path_P)

        // parse path from the end -> always use push_front
        for (std::list<int>::reverse_iterator i = path_P.rbegin(); i != path_P.rend(); ++i) {

            if (is_new_component_of_path_P) {
                current_end_P_index = *i; // we are starting a new component path. Change the path_end.
                // add the path_end at the front of the current component path ->i.e add it as the last element
                current_component_of_path_P.push_front(current_end_P_index);
                is_new_component_of_path_P = false; // reset flag
                continue; // skip iteration
            }

            if (intervalArray_c[*i].right()->value() > intervalArray_c[current_end_P_index].right()->value()) { // if (p_i > end(path_P))
                // store p_i as c_i. At the front of set C(P). That way at the end of the loop C(p) = {c_1, c_2, ... c_k}
                setC_of_path_P.push_front(*i);

                components_of_path_P.push_front(current_component_of_path_P); // store component

                current_component_of_path_P.clear(); // reset the current path
                is_new_component_of_path_P = true; // mark next iteration to start a new path.

            } else {
                // add *i at the front of the current component path
                current_component_of_path_P.push_front(*i);
            }
        }
        if (!current_component_of_path_P.empty())  {
            components_of_path_P.push_front(current_component_of_path_P); // store latest component
        }

        log_msg("printing path_P", llevel::INFO);
        log_data(path_P, llevel::DEBUG);

        log_msg("printing setC_of_path_P", llevel::INFO);
        log_data(setC_of_path_P, llevel::DEBUG);

        log_msg("printing components_of_path_P", llevel::INFO);
        log_data(components_of_path_P, llevel::DEBUG);

        /* here we have path_P both as path_P
         * and as P = P_1 -> c_1 -> ... -> c_k -> P_k+1
         * stored in list<int> path_P and
         * list<list<int>> components_of_path_P together with list<int> setC_of_path_P */

        if (components_of_path_P.size() != setC_of_path_P.size() + 1) {
           log_msg("|components_of_path_P| != |C(P) + 1|", llevel::ERROR);
            break;
        }

        /* step 6:  let arc_u = end(path_P_1) and arc_up end(path_P)
         *  where path_P_1 is the first component of path_P. */
        log_msg("/* step 6:  let arc_u = end(path_P_1) and arc_up end(path_P) where path_P_1 is the first component of path_P. */", llevel::INFO);

        int end_of_path_P_1_index = components_of_path_P.front().back(); // from components get the first element -> path_P_1.
        int end_of_path_P_index = path_P.back();

        int arc_u_index = intervalArray_c[end_of_path_P_1_index].getMp_originalArc()->tail_order_index(); //index of corresponding arc in mp_arcsArray
        int arc_up_index = intervalArray_c[end_of_path_P_index].getMp_originalArc()->tail_order_index(); //index of corresponding arc in mp_arcsArray

        log_msg("end_of_path_P_1_index = " + std::to_string(end_of_path_P_1_index), llevel::DEBUG);
        log_msg("end_of_path_P_index = " + std::to_string(end_of_path_P_index), llevel::DEBUG);
        log_msg("arc_u_index = " + std::to_string(arc_u_index), llevel::DEBUG);
        log_msg("arc_up_index = " + std::to_string(arc_up_index), llevel::DEBUG);

        /* step 7:  compute set B_p(tail(arc_up)) and check if arc_m is included */
        log_msg("/* step 7:  compute set B_p(tail(arc_up)) and check if arc_m is included */", llevel::INFO);

        int tail_arc_up_val = mp_arcsArray[arc_up_index].tail()->value();
        int Bp_t_of_up_size;
        compute_set_Bp(tail_arc_up_val, set_Bp_tail_of_up_indexes, Bp_t_of_up_size);

        bool ham_cycle_found_on7 = false;
        // construct cycle
        std::list<int> possible_cycle_on7;
        for (int i = 0; i < Bp_t_of_up_size; i++) {

            if (set_Bp_tail_of_up_indexes[i] == index_arc_m) {
                log_msg("HCArcsManager::runAlgorithm(): on step 7 found Hamiltonian cycle of G(F)", llevel::OUTPUT);

                for (std::list<int>::iterator it = path_P.begin(); it != path_P.end(); it++) {
                    int corresponding_arc_index = intervalArray_c[*it].getMp_originalArc()->tail_order_index();
                    possible_cycle_on7.push_back(corresponding_arc_index);
                }
                int start_path_P_index = intervalArray_c[path_P.front()].getMp_originalArc()->tail_order_index();
                possible_cycle_on7.push_back(start_path_P_index);

                log_data(possible_cycle_on7, llevel::DEBUG);

                ham_cycle_found_on7 = true;
                break;
            }
        }

        // send output to view. If cycle is empty, more outputs will come.
        if (m_toView) produceOutput(outputCase::CASE_7,
                                    intervalArray_c, m_num_of_arcs,
                                    mp_arcsArray, m_num_of_arcs,
                                    path_P, corresp_arc_indexes_toPathP,
                                    possible_cycle_on7);

        if (ham_cycle_found_on7) {

            bool output_correctness_ok = true;
            if (!check_path_intersection(mp_arcsArray, m_num_of_arcs, possible_cycle_on7)) output_correctness_ok = false;

            if (!output_correctness_ok) {
                log_msg("HCArcsManager::runAlgorithm(): found incorrect path in output", llevel::ERROR);
                break;
            }

            ok = true;
            break;
        }


        /* step 8-9: compute F_cc = F_cc( 0 , tail(arc_u)) and consturct interval model I_cc from ArcToI*/
        log_msg("/* step 8-9: compute F_cc = F_cc( 0 , tail(arc_u)) and consturct interval model I_cc from ArcToI*/", llevel::INFO);

        int tail_u_val = mp_arcsArray[arc_u_index].tail()->value(); // get tail_u;

        // initialize data sets here
        intervalArray_cc = new Interval[m_num_of_arcs];
        intervalPtrArray_cc = new Interval*[m_num_of_arcs];

        log_dashedLine(); // ---
        ArcToI_ok = false;

        arcToIManager.resetManager(); // "reset" manager
        arcToIManager.setNumOfArcs(m_num_of_arcs);
        ArcToI_ok = arcToIManager.runAlgorithm( mp_arcsArray, mp_arcsPtrArray, m_num_of_arcs,
                                                tail_u_val,
                                                nullptr, 0, direction::CC,
                                                intervalArray_cc, intervalPtrArray_cc);

        if (!ArcToI_ok) {
            log_msg("HCArcsManager::runAlgorithm(): ArcToI failed to run with counterclockwise direction", llevel::ERROR);
            break;
        }
        log_dashedLine(); // ---

        // match these Intervals (intervalArray_cc) indexes to their original arcs. To form a 2-way connection

        for (int i = 0; i < m_num_of_arcs; i++) {
            intervalArray_cc[i].getMp_originalArc()->setCorr_cc_int_index(i);
        }

        /* step 10: call procedure DOPC to obtain path Q from I_cc */
        log_msg("/* step 10: call procedure DOPC to obtain path Q from I_cc */", llevel::INFO);

        log_dashedLine(); // ---
        bool dopc_for_Q_ok = false;
        dopcManager.resetManager(); // "reset" manager
        dopcManager.setNumOfIntervals(m_num_of_arcs);

        pathCover_Q = new std::list<std::list<int>>();

        dopc_for_Q_ok = dopcManager.runAlgorithm(intervalArray_cc, intervalPtrArray_cc, m_num_of_arcs, *pathCover_Q);

        if (!dopc_for_Q_ok) {
            log_msg("HCArcsManager::runAlgorithm(): DOPC failed to run", llevel::ERROR);
            break;
        }
        log_dashedLine(); // ---


        // construct path Q
        std::list<int> path_Q = pathCover_Q->front();
        std::list<int> corresp_arc_indexes_toPathQ; // the corresponding arc indexes. Needed to present output to view.

        for (std::list<int>::iterator it = path_Q.begin(); it != path_Q.end(); it++) {
            int corresponding_arc_index = intervalArray_cc[*it].getMp_originalArc()->tail_order_index();
            corresp_arc_indexes_toPathQ.push_back(corresponding_arc_index);
        }

        /* step 11: check if path Q is a Hamiltonian path of G(I_cc).
         * This can be done by checking if |PC(I_cc)| > 1 */
        log_msg("/* step 11: check if path Q is a Hamiltonian path of G(I_cc). This can be done by checking if |PC(I_cc)| > 1 */", llevel::INFO);
        log_msg("printing path cover of I_cc in I_cc indexes", llevel::INFO);
        log_data(*pathCover_Q, llevel::DEBUG);

        if (pathCover_Q->size() > 1) {
            log_msg("HCArcsManager::runAlgorithm(): on step 11 found that G(F) has no Hamiltonian cycle", llevel::OUTPUT);
            // send output to view.
            std::list<int> empty_cycle;
            if (m_toView) produceOutput(outputCase::CASE_11,
                                        intervalArray_cc, m_num_of_arcs,
                                        mp_arcsArray, m_num_of_arcs,
                                        path_Q, corresp_arc_indexes_toPathQ, empty_cycle);
            ok = true;
            break;
        }

        /* step 12: let arc_z and arc_w be the arcs corresponding to start(path_Q) and end(path_Q), respectively */
        log_msg("/* step 12: let arc_z and arc_w be the arcs corresponding to start(path_Q) and end(path_Q), respectively */", llevel::INFO);


        int start_of_path_Q_index = path_Q.front();
        int end_of_path_Q_index = path_Q.back();

        int arc_z_index = intervalArray_cc[start_of_path_Q_index].getMp_originalArc()->tail_order_index(); //index of corresponding arc in mp_arcsArray
        int arc_w_index = intervalArray_cc[end_of_path_Q_index].getMp_originalArc()->tail_order_index(); //index of corresponding arc in mp_arcsArray

        log_msg("arc_z_index = " + std::to_string(arc_z_index), llevel::DEBUG);
        log_msg("arc_w_index = " + std::to_string(arc_w_index), llevel::DEBUG);

        /* step 13: check if arc_z and arc_w intersect */
        log_msg("/* step 13: check if arc_z and arc_w intersect */", llevel::INFO);

        bool ham_cycle_found_on13 = false;
        std::list<int> possible_cycle_on13;

        if (mp_arcsArray[arc_z_index].intersects_arc(mp_arcsArray[arc_w_index])) {

            log_msg("HCArcsManager::runAlgorithm(): on step 13 found Hamiltonian cycle of G(F)", llevel::OUTPUT);

            for (std::list<int>::iterator it = path_Q.begin(); it != path_Q.end(); it++) {
                int corresponding_arc_index = intervalArray_cc[*it].getMp_originalArc()->tail_order_index();
                possible_cycle_on13.push_back(corresponding_arc_index);
            }
            int start_path_Q_index = intervalArray_cc[path_Q.front()].getMp_originalArc()->tail_order_index();
            possible_cycle_on13.push_back(start_path_Q_index);

            log_data(possible_cycle_on13, llevel::DEBUG);

            ham_cycle_found_on13 = true;
        }

        // send output to view.
        if (m_toView) produceOutput(outputCase::CASE_13,
                                    intervalArray_cc, m_num_of_arcs,
                                    mp_arcsArray, m_num_of_arcs,
                                    path_Q, corresp_arc_indexes_toPathQ, possible_cycle_on13);

        if (ham_cycle_found_on13) {
            bool output_correctness_ok = true;
            if (!check_path_intersection(mp_arcsArray, m_num_of_arcs, possible_cycle_on13)) output_correctness_ok = false;

            if (!output_correctness_ok) {
                log_msg("HCArcsManager::runAlgorithm(): found incorrect path in output", llevel::ERROR);
                break;
            }

            ok = true;
            break;
        }


        /* step 14: let set_R = { arc_r in F | arc_r in B_p(head(arc_w)) - F(L(path_Q)) } */
        log_msg("/* step 14: let set_R = { arc_r in F | arc_r in B_p(head(arc_w)) - F(L(path_Q)) } */", llevel::INFO);

        // compute B_p(head(arc_w))
        int head_arc_w_val = mp_arcsArray[arc_w_index].head()->value();
        int Bp_head_of_w_size;
        compute_set_Bp(head_arc_w_val, set_Bp_head_of_w_indexes, Bp_head_of_w_size);

        // construct L(path_Q) = {interval_x in I_cc | interval_x in path_Q and interval_x > end(path_Q) }
        std::list<int> setL_of_path_Q;

        for (std::list<int>::iterator it = path_Q.begin(); it != path_Q.end(); it++) {

            if (intervalArray_cc[*it].right()->value() > intervalArray_cc[end_of_path_Q_index].right()->value()) {
                setL_of_path_Q.push_back(*it);
            }
        }
        log_msg("print setL_of_path_Q", llevel::INFO);
        log_data(setL_of_path_Q, llevel::DEBUG);

        // get the corresponding indexes to arcs to L(path_Q). i.e. F(L(path_Q))

        // match them to a binary array
        int FL_of_path_Q_indexes_barray[m_num_of_arcs] = {0};

        for (std::list<int>::iterator it = setL_of_path_Q.begin(); it != setL_of_path_Q.end(); it++) {

            int arc_index = *it;
            FL_of_path_Q_indexes_barray[arc_index] = 1;
        }

        // create binary table for Bp_head_of_w
        int Bp_head_of_w_indexes_barray[m_num_of_arcs] = {0};
        for (int i = 0; i < Bp_head_of_w_size; i++) {

            int arc_index = set_Bp_head_of_w_indexes[i];
            Bp_head_of_w_indexes_barray[arc_index] = 1;
        }

        // create set_R = B_p(head(arc_w)) - F(L(path_Q))
        int set_R_barray[m_num_of_arcs] = {0};

        for (int i = 0; i < m_num_of_arcs; i++) {

            if (Bp_head_of_w_indexes_barray[i] == 1) {

                if (FL_of_path_Q_indexes_barray[i] == 1) {
                    set_R_barray[i] = 0; // B_p(head(arc_w)) - F(L(path_Q))
                } else {
                    set_R_barray[i] = 1;
                }
            }
        }

        std::list<int> set_R;
        for (int i = 0; i < m_num_of_arcs; i++) {
            if (set_R_barray[i] == 1) {
                set_R.push_back(i); // now (list) set_R holds the indexes of the arcs in set R
            }
        }
        // set_R has size = Delta. So operation set_R.remove(K) costs O(Delta). As a sidenote, even O(n) cost here is fine.

        log_msg("print set_R = { arc_r in F | arc_r in B_p(head(arc_w)) - F(L(path_Q)) }", llevel::INFO);
        log_data(set_R, llevel::DEBUG);


        bool dopc_for_Q_s_ok = true; // needed for double break. Must start as true
        bool ham_cycle_found_on19 = false; // needed for double break. Must start as false
        std::list<int> possible_cycle_on19;

        /* step 15: while (set_R != empty) */
        log_msg("/* step 15: while (set_R != empty) loop start */", llevel::INFO);

        while (!set_R.empty()) {

            /* step 16: let arc_s in set_R be an arc such that right(I_cc(arc_s)) is the smallest in I_cc(R) */
            log_msg("/* step 16: let arc_s in set_R be an arc such that right(I_cc(arc_s)) is the smallest in I_cc(R) */", llevel::INFO);

            // find corresponding_cc_interval with minimum right().val() in I_cc(R)
            int arc_s_index = -1;
            int min_right_cc_interval_index = mp_arcsArray[set_R.front()].corr_cc_int_index();
            int min_right_cc_interval_val = intervalArray_cc[min_right_cc_interval_index].right()->value();
            for (std::list<int>::iterator it = set_R.begin(); it != set_R.end(); it++) {

                int right_cc_interval_index = mp_arcsArray[*it].corr_cc_int_index();
                int right_cc_interval_val = intervalArray_cc[right_cc_interval_index].right()->value();

                if (right_cc_interval_val < min_right_cc_interval_val) {
                    min_right_cc_interval_val = right_cc_interval_val;
                    min_right_cc_interval_index = right_cc_interval_index;
                }
            }
            arc_s_index = intervalArray_cc[min_right_cc_interval_index].getMp_originalArc()->tail_order_index();

            log_msg("print arc_s_index = " + std::to_string(arc_s_index), llevel::DEBUG);
            log_msg("print min_right_cc_interval_index = " + std::to_string(min_right_cc_interval_index), llevel::DEBUG);


            /* step 17: I_s = I_cc - I_cc(arc_s) */
            log_msg("/* step 17: I_s = I_cc - I_cc(arc_s) */", llevel::INFO);

            // copy the intervals and remove mp_arcsArray[arc_s_index].corr_cc_int_index()
            int interval_s_cc_Rightindex =  mp_arcsArray[arc_s_index].corr_cc_int_index();
            int interval_s_cc_Leftindex = intervalArray_cc[interval_s_cc_Rightindex].left_order_index();


            // cleanup previous I_s array
            if (I_s_Array) {
                delete [] I_s_Array;
                I_s_Array = nullptr;
            }

            // cleanup previous I_s_ptrArray array
            if (I_s_ptrArray) {
                delete [] I_s_ptrArray;
                I_s_ptrArray = nullptr;
            }


            int I_s_size = m_num_of_arcs -1;
            I_s_Array = new Interval[I_s_size];
            int i_s_counter = 0;

            for (int i =0; i < m_num_of_arcs; i++) {

                if (i == interval_s_cc_Rightindex) {
                    continue;

                } else {
                    I_s_Array[i_s_counter] = Interval(intervalArray_cc[i]);
                    I_s_Array[i_s_counter].setRight_order_index(i_s_counter);
                    I_s_Array[i_s_counter].left()->setParent(&I_s_Array[i_s_counter]);
                    I_s_Array[i_s_counter].right()->setParent(&I_s_Array[i_s_counter]);

                    // find the correct left order index
                    int old_left_order_index = intervalArray_cc[i].left_order_index();

                    if (old_left_order_index >= interval_s_cc_Leftindex) {
                        old_left_order_index -= 1;
                    }
                    I_s_Array[i_s_counter].setLeft_order_index(old_left_order_index);
                    i_s_counter ++;
                }
            }

            I_s_ptrArray = new Interval*[I_s_size];

            for (int i = 0; i < I_s_size; i ++) {
                int left_index = I_s_Array[i].left_order_index();
                I_s_ptrArray[left_index] = &I_s_Array[i];
            }

            log_msg("print I_s intervals", llevel::INFO);
            log_data(I_s_Array, I_s_size, llevel::DEBUG);
            log_data(I_s_ptrArray, I_s_size, llevel::DEBUG);


            /* step 18: call procedure DOPC to obtain path Q_s from I_s */
            log_msg("/* step 18: call procedure DOPC to obtain path Q_s from I_s */", llevel::INFO);

            log_dashedLine(); // ---
            dopcManager.resetManager(); // "reset" manager
            dopcManager.setNumOfIntervals(I_s_size);

            // cleanup previous PC
            if (pathCover_Q_s) {
                delete pathCover_Q_s;
                pathCover_Q_s = nullptr;
            }
            pathCover_Q_s = new std::list<std::list<int>>();

            dopc_for_Q_s_ok = dopcManager.runAlgorithm(I_s_Array, I_s_ptrArray, I_s_size, *pathCover_Q_s);

            if (!dopc_for_Q_s_ok) {
                // break from while loop
                break;
            }
            log_dashedLine(); // ---

            log_msg("print pathCover_Q_s. Warning that indexes in this pathCover match indexes in I_s", llevel::INFO);
            log_data(*pathCover_Q_s, llevel::DEBUG);


            /* step 19: check if Q_s is the Hamiltonian path of G(I_s)
             * and arc_s intersects arc F(end(Q_s)) */
            log_msg("/* step 19: check if Q_s is the Hamiltonian path of G(I_s) and arc_s intersects arc F(end(Q_s)) */", llevel::INFO);

            std::list<int> path_Q_s = pathCover_Q_s->front();
            /* indexes of intervals inside path_Q_s do not match with intervals_cc.
             * Parse them to correct the path.
             * Also, we only care about right endpoint indexes, since those are stored inside the path. */

            for (std::list<int>::iterator it = path_Q_s.begin(); it != path_Q_s.end(); it++) {
                if (*it >= interval_s_cc_Rightindex ) {
                    *it = *it + 1;
                }
            }
            std::list<int> corresp_arc_indexes_toPathQ_s;
            for (std::list<int>::iterator it = path_Q_s.begin(); it != path_Q_s.end(); it++) {
                int corresponding_arc_index = intervalArray_cc[*it].getMp_originalArc()->tail_order_index();
                corresp_arc_indexes_toPathQ_s.push_back(corresponding_arc_index);
            }
            log_msg("print path_Q_s corresponding arc indexes", llevel::INFO);
            log_data(corresp_arc_indexes_toPathQ_s, llevel::DEBUG);


            int arc_F_end_Q_s_index = intervalArray_cc[path_Q_s.back()].getMp_originalArc()->tail_order_index();

            if (pathCover_Q_s->size() == 1 &&
                    mp_arcsArray[arc_s_index].intersects_arc(mp_arcsArray[arc_F_end_Q_s_index])) {


                log_msg("HCArcsManager::runAlgorithm(): on step 19 found Hamiltonian cycle of G(F)", llevel::OUTPUT);

                // cycle is s -> Q_s -> s

                possible_cycle_on19.push_back(arc_s_index);
                for (std::list<int>::iterator it = path_Q_s.begin(); it != path_Q_s.end(); it++) {
                    int corresponding_arc_index = intervalArray_cc[*it].getMp_originalArc()->tail_order_index();
                    possible_cycle_on19.push_back(corresponding_arc_index);
                }
                possible_cycle_on19.push_back(arc_s_index);
                log_data(possible_cycle_on19, llevel::DEBUG);

                ham_cycle_found_on19 = true;
                // not break; here, to send output to view first

            } else {

                /* step 20: let R = R - {arc_s} */
                log_msg("/* step 20: let R = R - {arc_s} */", llevel::INFO);

                set_R.remove(arc_s_index);

                log_msg("print set_R", llevel::INFO);
                log_data(set_R, llevel::DEBUG);
            }

            // will reach here in both cases. With or without a cycle on 19.
            // send output to view.
            if (m_toView) produceOutput(outputCase::CASE_19,
                                        I_s_Array, I_s_size,
                                        mp_arcsArray, m_num_of_arcs,
                                        path_Q_s, corresp_arc_indexes_toPathQ_s, possible_cycle_on19);

            if (ham_cycle_found_on19) {
                break; // exit the loop
            }


        } // end while(set_R != empty)

        if (!dopc_for_Q_s_ok) {
            log_msg("HCArcsManager::runAlgorithm(): DOPC failed to run", llevel::ERROR);
            break;
        }

        if (ham_cycle_found_on19) {

            bool output_correctness_ok = true;
            if (!check_path_intersection(mp_arcsArray, m_num_of_arcs, possible_cycle_on19)) output_correctness_ok = false;

            if (!output_correctness_ok) {
                log_msg("HCArcsManager::runAlgorithm(): found incorrect path in output", llevel::ERROR);
                break;
            }

            ok = true;
            break;
        }


        /* step 21: check if G(I_cc) is Hamiltonian.
         * We can call HCIntervalsManager */
        log_msg("/* step 21: check if G(I_cc) is Hamiltonian. We can call HCIntervalsManager */", llevel::INFO);

        log_dashedLine(); //---
        HCIntervalsManager hcintervalsManager(m_num_of_arcs);

        bool hcintervals_ok = false;
        intervalsCycle = new std::list<int>;

        hcintervals_ok = hcintervalsManager.runAlgorithm(intervalArray_cc, m_num_of_arcs, *pathCover_Q, *intervalsCycle);

        if (!hcintervals_ok) {
            log_msg("HCArcsManager::runAlgorithm(): HCIntervals failed to run", llevel::ERROR);
            break;
        }
        log_dashedLine(); //---

        if (!intervalsCycle->empty()) {
            log_msg("HCArcsManager::runAlgorithm(): on step 21 found Hamiltonian cycle of G(F)", llevel::OUTPUT);


            std::list<int> cycle_on21;
            for (std::list<int>::iterator it = intervalsCycle->begin(); it != intervalsCycle->end(); it++) {
                int corresponding_arc_index = intervalArray_cc[*it].getMp_originalArc()->tail_order_index();
                cycle_on21.push_back(corresponding_arc_index);
            }

            log_data(cycle_on21, llevel::DEBUG);

            bool output_correctness_ok = true;
            if (!check_path_intersection(mp_arcsArray, m_num_of_arcs, cycle_on21)) output_correctness_ok = false;

            if (!output_correctness_ok) {
                log_msg("HCArcsManager::runAlgorithm(): found incorrect path in output", llevel::ERROR);
                break;
            }

            // steps 21 and 22 are mutualy exclusive.
            // send output to view.
            if (m_toView) produceOutput(outputCase::CASE_21,
                                        intervalArray_cc, m_num_of_arcs,
                                        mp_arcsArray, m_num_of_arcs,
                                        *intervalsCycle, cycle_on21, cycle_on21); // cycle_on21 is the corresponding arc indexes path

        } else {
            /* step 22: final output, no hamilton cycle */
            log_msg("/* step 22: final output, no hamilton cycle */", llevel::INFO);
            log_msg("HCArcsManager::runAlgorithm(): on step 22 found that G(F) has no Hamiltonian cycle", llevel::OUTPUT);

            // send output to view.
            std::list<int> empty_path;
            if (m_toView) produceOutput(outputCase::CASE_22,
                                        intervalArray_cc, m_num_of_arcs,
                                        mp_arcsArray, m_num_of_arcs,
                                        empty_path, empty_path, empty_path); // hcintervalsManager found no cycle, just draw nothing
        }

        ok = true;
    } while(0);

    log_msg("HCArcsManager::runAlgorithm(): end", llevel::INFO);

    // cleanups

    // cleanup B_a(m)
    if (set_Ba_indexes) {
        delete [] set_Ba_indexes;
        set_Ba_indexes = nullptr;
    }

    // cleanup B_p(tail(arc_up))
    if (set_Bp_tail_of_up_indexes) {
        delete [] set_Bp_tail_of_up_indexes;
        set_Bp_tail_of_up_indexes = nullptr;
    }

    // cleanup B_p(head(arc_w))
    if (set_Bp_head_of_w_indexes) {
        delete [] set_Bp_head_of_w_indexes;
        set_Bp_head_of_w_indexes = nullptr;
    }

    // cleanup interval arrays
    if (intervalArray_c) {
        delete [] intervalArray_c;
        intervalArray_c = nullptr;
    }

    if (intervalPtrArray_c) {
        delete [] intervalPtrArray_c;
        intervalPtrArray_c = nullptr;
    }

    if (intervalArray_cc) {
        delete [] intervalArray_cc;
        intervalArray_cc = nullptr;
    }

    if (intervalPtrArray_cc) {
        delete [] intervalPtrArray_cc;
        intervalPtrArray_cc = nullptr;
    }

    if (I_s_Array) {
        delete [] I_s_Array;
        I_s_Array = nullptr;
    }

    if (I_s_ptrArray) {
        delete [] I_s_ptrArray;
        I_s_ptrArray = nullptr;
    }

    // cleanup path cover list
    if (pathCover_P) {
        delete pathCover_P;
        pathCover_P = nullptr;
    }

    if (pathCover_Q) {
        delete pathCover_Q;
        pathCover_Q = nullptr;
    }

    if (pathCover_Q_s) {
        delete pathCover_Q_s;
        pathCover_Q_s = nullptr;
    }

    if (intervalsCycle) {
        delete intervalsCycle;
        intervalsCycle = nullptr;
    }

    return ok;
}

void HCArcsManager::setOutputFunc(const HCArcsOutputFunction &outputFunc)
{
    m_outputFunc = outputFunc;
}

void HCArcsManager::setInputDataToViewFunc(const HCArcsOutputFunction &dataToViewFunc)
{
    m_inputDataToViewFunc = dataToViewFunc;
}

/* returns the index of the arc with the minimum length. O(N) */
int HCArcsManager::findIndexOfArcContainingNoOther(Arc *arcsArray, int num_of_arcs) const
{
    log_msg("findIndexOfArcContainingNoOther: start" , llevel::INFO);
    int max_ep = 2 * num_of_arcs;
    int min_len = arcsArray[0].calculate_length(max_ep);
    int min_len_index = 0;

    for (int i = 1; i < num_of_arcs; i ++) {
        int len = arcsArray[i].calculate_length(max_ep);

        if (len == 1 || // no endpoint in between -> -> does not contain any other arc
                len == 2) { // only one endpoint in between -> does not contain any other arc
            min_len_index = i;
            break;
        }

        if (len < min_len) {
            min_len = len;
            min_len_index = i; // minimum length arc, is sure to not contain any other arc.
        }
    }

    log_msg("findIndexOfArcContainingNoOther: found arc with index " + std::to_string(min_len_index), llevel::DEBUG);
    log_msg("findIndexOfArcContainingNoOther: end" , llevel::INFO);
    return  min_len_index;
}

/* set Bp(point p) = all arcs containing point p, O(N) */
void HCArcsManager::compute_set_Bp(int point_p, int *&set_Bp_out, int &setSize_out) const
{
    log_msg("compute_set_Bp: start with point_p value = " + std::to_string(point_p), llevel::INFO);

    int mark_Bp[m_num_of_arcs] = {0};
    setSize_out = 0;

    // mark arcs that contain point p and count them
    for (int i = 0; i < m_num_of_arcs; i++) {
        if (mp_arcsArray[i].contains_point(point_p)) {
            mark_Bp[i] = 1;
            setSize_out ++;
        }
    }

    // create set Bp
    set_Bp_out = new int[setSize_out];
    int j = 0;
    for (int i = 0; i < m_num_of_arcs; i ++ ) {
        if (mark_Bp[i] == 1) {
            set_Bp_out[j] = i;
            j++;
        }
    }
    log_data(set_Bp_out, setSize_out, llevel::DEBUG);
    log_msg("compute_set_Bp: end" , llevel::INFO);
}
/* set Ba(arc a) = all arcs containing arc a, O(N) */
void HCArcsManager::compute_set_Ba(const Arc &arc_a, int *&set_Ba_out, int &setSize_out) const
{
    log_msg("compute_set_Ba: start with index of arc_a : " + std::to_string(arc_a.tail_order_index()) , llevel::INFO);

    int mark_Ba[m_num_of_arcs] = {0};
    setSize_out = 0;

    // mark arcs that contain arc_a and count them
    for (int i = 0; i < m_num_of_arcs; i++) {
        if (mp_arcsArray[i].contains_arc(arc_a)) {
            mark_Ba[i] = 1;
            setSize_out ++;
        }
    }

    // create set Ba
    set_Ba_out = new int[setSize_out];
    int j = 0;
    for (int i = 0; i < m_num_of_arcs; i ++ ) {
        if (mark_Ba[i] == 1) {
            set_Ba_out[j] = i;
            j++;
        }
    }
    log_data(set_Ba_out, setSize_out, llevel::DEBUG);
    log_msg("compute_set_Ba: end" , llevel::INFO);
}

void HCArcsManager::resetManager()
{
    cleanup();
}

void HCArcsManager::init()
{
    m_num_of_arcs = 0;
    m_initialized = false;
    mp_arcsArray = nullptr;
    mp_arcsPtrArray = nullptr;

    m_toView = false;
    m_outputFunc = nullptr;
}

void HCArcsManager::cleanup()
{
    if (mp_arcsArray) {
        delete [] mp_arcsArray;
        mp_arcsArray = nullptr;
    }
    if (mp_arcsPtrArray) {
        delete [] mp_arcsPtrArray;
        mp_arcsPtrArray = nullptr;
    }
}

bool HCArcsManager::checkInput(Arc *arcsArray, Arc **leftArcsPtrArray, int num_of_arcs)
{
    bool ok = false;

    do {

        log_msg("HCArcsManager::checkInput(): start", llevel::INFO);

        if (arcsArray == nullptr) {
            log_msg("HCArcsManager::checkInput(): null array pointer in this function input", llevel::ERROR);
            break;
        }
        if (leftArcsPtrArray == nullptr) {
            log_msg("HCArcsManager::checkInput(): null array pointer in this function input", llevel::ERROR);
            break;
        }
        if (num_of_arcs != m_num_of_arcs) {
            log_msg("HCArcsManager::checkInput(): size != m_num_of_arcs", llevel::ERROR);
            break;
        }
        if (m_num_of_arcs < 3) {
            log_msg("HCArcsManager::checkInput(): input size < 3. Can't find hamiltonian cycle with only 2 arcs", llevel::ERROR);
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
            log_msg("HCArcsManager::checkInput(): input has arc with endpoint out of bounds", llevel::ERROR);
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
            log_msg("HCArcsManager::checkInput(): input does not have correct endpoint ordering", llevel::ERROR);
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
            log_msg("arcsArray[0].tail_order_index() != 0", llevel::ERROR);
            ascending_tail_ordering_ok = false;
        }

        if (!ascending_tail_ordering_ok) {
            log_msg("HCArcsManager::checkInput(): input is not ordered based on tails (ascending) ", llevel::ERROR);
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
            log_msg("HCArcsManager::checkInput(): input is not ordered based on heads (ascending) ", llevel::ERROR);
            break;
        }

        log_msg("HCArcsManager::checkInput(): end", llevel::INFO);
        ok = true;
    } while(0);

    return ok;
}

bool HCArcsManager::initializeMembers(Arc *arcsArray, Arc **leftArcsPtrArray, int num_of_arcs)
{
    bool ok = false;

    do {

        log_msg("HCArcsManager::initializeMembers(): start", llevel::INFO);

        if (arcsArray == nullptr) {
            log_msg("HCArcsManager::initializeMembers(): null array pointer in this function input", llevel::ERROR);
            break;
        }
        if (leftArcsPtrArray == nullptr) {
            log_msg("HCArcsManager::initializeMembers(): null array pointer in this function input", llevel::ERROR);
            break;
        }
        if (num_of_arcs != m_num_of_arcs) {
            log_msg("HCArcsManager::initializeMembers(): size != m_num_of_arcs", llevel::ERROR);
            break;
        }

        mp_arcsArray = new Arc[m_num_of_arcs];
        mp_arcsPtrArray = new Arc*[m_num_of_arcs];

        for (int i=0; i < m_num_of_arcs; i++) {
            mp_arcsArray[i] = std::move(arcsArray[i]);
            int left_index = mp_arcsArray[i].head_order_index();
            mp_arcsPtrArray[left_index] = &mp_arcsArray[i];
        }

        log_msg("HCArcsManager::initializeMembers(): end", llevel::INFO);
        ok = true;
    } while(0);

    return ok;
}

void HCArcsManager::produceOutput(HCArcsManager::outputCase c, Interval *intervalArray, int num_of_intervals, Arc *arcArray, int num_of_arcs, const std::list<int> &intervalIndexesPath, const std::list<int> &arcIndexesPath, const std::list<int> &arcIndexesCycle)
{
    if (!m_outputFunc) {
        log_msg("no output callback function specified. Continuing as is, and outputing on console", llevel::ERROR);
    } else {

        HCArcsOutput out(c,
                         intervalArray, num_of_intervals,
                         arcArray, num_of_arcs,
                         intervalIndexesPath,
                         arcIndexesPath,
                         arcIndexesCycle);
        // copy the temp object, it takes O(n), it is fine on every outputCase.
        // give it to callback and continue the algorithm
        m_outputFunc(out);
    }
}

// ---------------------------

HCArcsOutput::HCArcsOutput()
{
    init();
}

HCArcsOutput::HCArcsOutput(HCArcsManager::outputCase c, Interval *intervalArray, int num_of_intervals, Arc *arcArray, int num_of_arcs, std::list<int> intervalIndexesPath, std::list<int> arcIndexesPath, std::list<int> arcIndexesCycle)
{
    init();

    m_case = c;

    m_num_of_intervals = num_of_intervals;
    if (intervalArray) {
        mp_intervalArray = new SimpleInterval[m_num_of_intervals];
        for (int i = 0; i < m_num_of_intervals; i++) {
            int left_val = intervalArray[i].left()->value();
            int right_val = intervalArray[i].right()->value();
            int order = i;

            mp_intervalArray[i] = SimpleInterval(left_val, right_val, order);
        }
    }

    m_num_of_arcs = num_of_arcs;
    if (arcArray) {
        mp_arcArray = new SimpleArc[m_num_of_arcs];
        for (int i = 0; i < m_num_of_arcs; i++) {
            int head_val = arcArray[i].head()->value();
            int tail_val = arcArray[i].tail()->value();
            int order = i;

            mp_arcArray[i] = SimpleArc(head_val, tail_val, order);
        }
    }

    m_intervalIndexesPath.assign(intervalIndexesPath.begin(), intervalIndexesPath.end());
    m_arcIndexesPath.assign(arcIndexesPath.begin(), arcIndexesPath.end());
    m_arcIndexesCycle.assign(arcIndexesCycle.begin(), arcIndexesCycle.end());
}

HCArcsOutput::~HCArcsOutput()
{
    if (mp_intervalArray) {
        delete [] mp_intervalArray;
        mp_intervalArray = nullptr;
    }
    if (mp_arcArray) {
        delete [] mp_arcArray;
        mp_arcArray = nullptr;
    }
}

void HCArcsOutput::init()
{
    m_case = HCArcsManager::outputCase::DEF_CASE;
    m_num_of_intervals = 0;
    m_num_of_arcs = 0;
    mp_intervalArray = nullptr;
    mp_arcArray = nullptr;
}
