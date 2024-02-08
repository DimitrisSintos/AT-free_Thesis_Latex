void HCArcsManager::testHCArcsExample1()
{
    int N  = 11;
    m_num_of_arcs = N;

    Arc *arcsArray = new Arc[m_num_of_arcs];
    Arc **leftArcsPtrArray = new Arc*[m_num_of_arcs];

    arcsArray[0] = Arc(18, 22);
    arcsArray[0].head()->set_order(17);
    arcsArray[0].tail()->set_order(21);
    arcsArray[1] = Arc(21, 1);
    arcsArray[1].head()->set_order(20);
    arcsArray[1].tail()->set_order(0);
    arcsArray[2] = Arc(8, 3);
    arcsArray[2].head()->set_order(7);
    arcsArray[2].tail()->set_order(2);
    arcsArray[3] = Arc(2, 4);
    arcsArray[3].head()->set_order(1);
    arcsArray[3].tail()->set_order(3);
    arcsArray[4] = Arc(5, 6);
    arcsArray[4].head()->set_order(4);
    arcsArray[4].tail()->set_order(5);
    arcsArray[5] = Arc(12, 7);
    arcsArray[5].head()->set_order(11);
    arcsArray[5].tail()->set_order(6);
    arcsArray[6] = Arc(20, 10);
    arcsArray[6].head()->set_order(19);
    arcsArray[6].tail()->set_order(9);
    arcsArray[7] = Arc(11, 14);
    arcsArray[7].head()->set_order(10);
    arcsArray[7].tail()->set_order(13);
    arcsArray[8] = Arc(15, 16);
    arcsArray[8].head()->set_order(14);
    arcsArray[8].tail()->set_order(15);
    arcsArray[9] = Arc(9, 17);
    arcsArray[9].head()->set_order(8);
    arcsArray[9].tail()->set_order(16);
    arcsArray[10] = Arc(13, 19);
    arcsArray[10].head()->set_order(12);
    arcsArray[10].tail()->set_order(18);


    leftArcsPtrArray[0] = &arcsArray[9];
    leftArcsPtrArray[1] = &arcsArray[0];
    leftArcsPtrArray[2] = &arcsArray[3];
    leftArcsPtrArray[3] = &arcsArray[1];
    leftArcsPtrArray[4] = &arcsArray[2];
    leftArcsPtrArray[5] = &arcsArray[6];
    leftArcsPtrArray[6] = &arcsArray[10];
    leftArcsPtrArray[7] = &arcsArray[5];
    leftArcsPtrArray[8] = &arcsArray[8];
    leftArcsPtrArray[9] = &arcsArray[4];
    leftArcsPtrArray[10] = &arcsArray[7];


    /* call merge sort to make sure ordering is correct */
    sort_runRecursively(arcsArray, 0, m_num_of_arcs - 1);
    sort_runRecursivelyPtrs(leftArcsPtrArray, 0, m_num_of_arcs - 1);

    /* fix tail && head order indexes */
    for (int i =0; i < m_num_of_arcs; i++) {
        arcsArray[i].setTail_order_index(i);
    }
    for (int i =0; i < m_num_of_arcs; i++) {
        leftArcsPtrArray[i]->setHead_order_index(i);
    }

    bool test_ok = runAlgorithm(arcsArray, leftArcsPtrArray, N);

    if (test_ok) {
        log_asteriskLine();
        log_msg("testRunAlgorithm = pass", llevel::INFO);
    } else {
        log_asteriskLine();
        log_msg("testRunAlgorithm = fail", llevel::INFO);
    }


    if (arcsArray) {
        delete [] arcsArray;
        arcsArray = nullptr;
    }

    if (leftArcsPtrArray) {
        delete [] leftArcsPtrArray;
        leftArcsPtrArray = nullptr;
    }
}
