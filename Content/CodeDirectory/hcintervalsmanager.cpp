#include "hcintervalsmanager.h"
#include "endpoint.h"
#include "hcarcs_object.h"

HCIntervalsManager::HCIntervalsManager()
{
    init();
}

HCIntervalsManager::HCIntervalsManager(int num_of_intervals)
{
    init();
    m_num_of_intervals = num_of_intervals;
}

HCIntervalsManager::~HCIntervalsManager()
{
    cleanup();
}

bool HCIntervalsManager::runAlgorithm(Interval *intervalArray, int num_of_intervals, const std::list<std::list<int>> &pathCover, std::list<int> &cycle_out)
{
    bool ok = false;

    do {
        log_msg("HCIntervalsManager::runAlgorithm(): start", llevel::INFO);

        m_initialized = checkInput(intervalArray, num_of_intervals, pathCover);

        if (!m_initialized) {
            log_msg("HCIntervalsManager::runAlgorithm(): failed to initialize manager", llevel::ERROR);
            break;
        }

        if (!cycle_out.empty()) {
            log_msg("HCIntervalsManager::runAlgorithm(): cycle is not empty" , llevel::ERROR);
            break;
        }

        // path_Z = PC(I).front();
        std::list<int> path_Z = pathCover.front();

        // construct L(path_Z) = {interval_x in I | interval_x in path_Z and interval_x > end(path_Z) }
        std::list<int> setL_of_path_Z;
        int end_of_path_Z_index = path_Z.back();

        for (std::list<int>::iterator it = path_Z.begin(); it != path_Z.end(); it++) {

            if (intervalArray[*it].right()->value() > intervalArray[end_of_path_Z_index].right()->value()) {
                setL_of_path_Z.push_back(*it);
            }
        }

        log_msg("HCIntervalsManager::runAlgorithm(): Printing input array", llevel::INFO);
        log_data(intervalArray, m_num_of_intervals, llevel::DEBUG);

        log_msg("HCIntervalsManager::runAlgorithm(): Printing input path cover", llevel::INFO);
        log_data(pathCover, llevel::DEBUG);

        log_msg("HCIntervalsManager::runAlgorithm(): Printing path Z", llevel::INFO);
        log_data(path_Z, llevel::DEBUG);


        // if (PC.size > 1 OR L(Z).size > 0 )
        //      Output no Hamiltonian cycle and return
        if (pathCover.size() > 1 || setL_of_path_Z.size() > 0) {
            log_msg("HCIntervalsManager::runAlgorithm(): no Hamiltonian cycle found in G(I)", llevel::OUTPUT);
            ok = true;
            break;
        }

        // make 100% sure:
        if (path_Z.size() != (size_t)m_num_of_intervals) {
            log_msg("HCIntervalsManager::runAlgorithm(): path_Z.size() != m_num_of_intervals" , llevel::ERROR);
            break;
        }

        /* here we are sure that PC(I).size == 1 .
         * So we can create an n-sized array for path_Z. */
        int path_ZArray[m_num_of_intervals] = {0};
        std::list<int>::iterator it_path_Z = path_Z.begin();
        for (int i = 0; i < m_num_of_intervals; i++) {

            path_ZArray[i] = *it_path_Z;
            it_path_Z ++;
        }

        // start paths P_1, P_2
        int z1 = path_ZArray[0];
        int z2 = path_ZArray[1];

        std::list<int> P_1;
        P_1.push_back(z1);
        std::list<int> P_2;
        P_2.push_back(z2);

        std::list<int> *P_asteriskPtr = nullptr; // "asterisk" is this <*> symbol
        std::list<int> *P_primePtr = nullptr; // "prime" is this <'> symbol

        log_msg("HCIntervalsManager::runAlgorithm(): start main loop", llevel::INFO);
        for (int i = 2; i < m_num_of_intervals; i++) {

            if (P_1.back() == path_ZArray[i-1]) {
                P_asteriskPtr = &P_1;
                P_primePtr = &P_2;
            } else {
                //P_2.back() == path_ZArray[i-1]
                P_asteriskPtr = &P_2;
                P_primePtr = &P_1;
            }

            // if (z_i is adjacent to end(P ′)) -> interval(z).intersects()

            int endP_prime_index = P_primePtr->back();

            if (intervalArray[path_ZArray[i]].intersects_interval(intervalArray[endP_prime_index])) {
                int z_i = path_ZArray[i];
                P_primePtr->push_back(z_i);
            } else {
                int z_i = path_ZArray[i];
                P_asteriskPtr->push_back(z_i);
            }

        }
        log_msg("HCIntervalsManager::runAlgorithm(): end main loop", llevel::INFO);

        log_msg("HCIntervalsManager::runAlgorithm(): Printing path P_1", llevel::INFO);
        log_data(P_1, llevel::DEBUG);
        log_msg("HCIntervalsManager::runAlgorithm(): Printing path P_2", llevel::INFO);
        log_data(P_2, llevel::DEBUG);

        std::list<int> *P_a = nullptr;
        std::list<int> *P_b = nullptr;

        if (P_1.back() == path_ZArray[m_num_of_intervals-1]) { // z_n = path_ZArray[m_num_of_intervals-1]
            P_a = &P_1;
            P_b = &P_2;
        } else {
            //P_2.back() == path_ZArray[m_num_of_intervals-1]
            P_a = &P_2;
            P_b = &P_1;
        }

        bool p_b_visits_z_n_1 = false;
        for (std::list<int>::iterator it = P_b->begin(); it != P_b->end(); it++) {

            if (*it == path_ZArray[m_num_of_intervals-2]) {
                p_b_visits_z_n_1 = true;
                break;
            }
        }

        if (p_b_visits_z_n_1) {
            // Output cycle =
            // path P_a
            for (std::list<int>::iterator it = P_a->begin(); it != P_a->end(); it++) {
                cycle_out.push_back(*it);
            }

            // plus reversed path P_b
            for (std::list<int>::reverse_iterator rit = P_b->rbegin(); rit != P_b->rend(); rit++) {
                cycle_out.push_back(*rit);
            }

            // at the end of the cycle add the 1st element of P_a
            cycle_out.push_back(P_a->front());
            log_msg("HCIntervalsManager::runAlgorithm(): found a Hamiltonian cycle in G(I)", llevel::OUTPUT);
            log_data(cycle_out, llevel::DEBUG);

            bool output_correctness_ok = true;
            if (!check_path_intersection(intervalArray, num_of_intervals, cycle_out)) output_correctness_ok = false;

            if (!output_correctness_ok) {
                log_msg("HCIntervalsManager::runAlgorithm(): found incorrect path in output", llevel::ERROR);
                break;
            }

        } else {
            //      Output no cycle
            log_msg("HCIntervalsManager::runAlgorithm(): no Hamiltonian cycle found in G(I)", llevel::OUTPUT);
        }

        log_msg("HCIntervalsManager::runAlgorithm(): end", llevel::INFO);
        ok = true;
    } while (0);


    return ok;
}

void HCIntervalsManager::resetManager()
{
    cleanup();
}

void HCIntervalsManager::init()
{
    m_num_of_intervals = 0;
    m_initialized = 0;
}

void HCIntervalsManager::cleanup()
{

}

bool HCIntervalsManager::checkInput(Interval *intervalArray, int num_of_intervals, const std::list<std::list<int> > &pathCover)
{
    bool ok = false;

    do {

        log_msg("HCIntervalsManager::checkInput(): start", llevel::INFO);
        if (intervalArray == nullptr) {
            log_msg( "HCIntervalsManager::checkInput(): null array pointer in this function input" , llevel::ERROR);
            break;
        }
        if (pathCover.empty()) {
            log_msg( "HCIntervalsManager::checkInput(): input pathCover is empty" , llevel::ERROR);
            break;
        }
        if (num_of_intervals != m_num_of_intervals) {
            log_msg( "HCIntervalsManager::checkInput(): size != m_num_of_arcs" , llevel::ERROR);
            break;
        }
        if (m_num_of_intervals < 3) {
            log_msg( "HCIntervalsManager::checkInput(): input size < 3. Can't find hamiltonian cycle with only 2 intervals" , llevel::ERROR);
            break;
        }

        bool inputPC_correctness_ok = true;
        for (std::list<std::list<int>>::const_iterator it = pathCover.begin(); it != pathCover.end(); it++) {

            if (!check_path_intersection(intervalArray, num_of_intervals, *it)) inputPC_correctness_ok = false;
        }

        if (!inputPC_correctness_ok) {
            log_msg("HCIntervalsManager::checkInput(): found incorrect path in input", llevel::ERROR);
            break;
        }

        // assuming following checks have been done by the algorithm that produced the path cover (e.g. DOPC)

        /* check if input is in ascending rightEp order*/
        /* check if input is in ascending leftEp order*/
        /* check if input has left_ep value[i] < right_ep value[i] */

        log_msg("HCIntervalsManager::checkInput(): end", llevel::INFO);
        ok = true;
    } while (0);


    return ok;
}
