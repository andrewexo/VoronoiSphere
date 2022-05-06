#include "../src/priqueue.h"
#include "../src/voronoi_event.h"
#include "../src/memblock.h"

#include "gtest/gtest.h"

TEST(PriQueueTests, TestInsertLinkedList)
{
    MemBlock<Increasing>* m_memBlocks;
    MemBlock<Increasing>* m_nextBlock;
    auto size = 5 * sizeof(MemBlock<Increasing>);
    m_nextBlock = m_memBlocks = (MemBlock<Increasing>*)malloc( size );
	int block = 0;

    new(&(m_nextBlock->priQueueNode)) PriQueueNode<Increasing>(block);
    new(&(m_nextBlock->skipNode)) SkipNode<Increasing>(block);
	block++;

    SkipNode<Increasing>* skipNode = &((m_nextBlock)->skipNode);
    PriQueueNode<Increasing>* pq1 = &((m_nextBlock)->priQueueNode);
    m_nextBlock++;

    new(getCircleEventFromSkipNode(skipNode)) CircleEvent<Increasing>(
		1.0, 0.0, glm::dvec3(0.0, 0.0, 0.0));

    CircleEvent<Increasing>* ce1 = getCircleEventFromSkipNode(skipNode);
    

    
    new(&(m_nextBlock->priQueueNode)) PriQueueNode<Increasing>(block);
    new(&(m_nextBlock->skipNode)) SkipNode<Increasing>(block);
	block++;

    skipNode = &((m_nextBlock)->skipNode);
    PriQueueNode<Increasing>* pq2 = &((m_nextBlock)->priQueueNode);
    m_nextBlock++;

    new(getCircleEventFromSkipNode(skipNode)) CircleEvent<Increasing>(
		2.0, 0.0, glm::dvec3(0.0, 0.0, 0.0));

    CircleEvent<Increasing>* ce2 = getCircleEventFromSkipNode(skipNode);



    new(&(m_nextBlock->priQueueNode)) PriQueueNode<Increasing>(block);
    new(&(m_nextBlock->skipNode)) SkipNode<Increasing>(block);
	block++;

    skipNode = &((m_nextBlock)->skipNode);
    PriQueueNode<Increasing>* pq3 = &((m_nextBlock)->priQueueNode);
    m_nextBlock++;

    new(getCircleEventFromSkipNode(skipNode)) CircleEvent<Increasing>(
		3.0, 0.0, glm::dvec3(0.0, 0.0, 0.0));

    CircleEvent<Increasing>* ce3 = getCircleEventFromSkipNode(skipNode);



    new(&(m_nextBlock->priQueueNode)) PriQueueNode<Increasing>(block);
    new(&(m_nextBlock->skipNode)) SkipNode<Increasing>(block);
	block++;

    skipNode = &((m_nextBlock)->skipNode);
    PriQueueNode<Increasing>* pq4 = &((m_nextBlock)->priQueueNode);
    m_nextBlock++;

    new(getCircleEventFromSkipNode(skipNode)) CircleEvent<Increasing>(
		4.0, 0.0, glm::dvec3(0.0, 0.0, 0.0));

    CircleEvent<Increasing>* ce4 = getCircleEventFromSkipNode(skipNode);
    

    new(&(m_nextBlock->priQueueNode)) PriQueueNode<Increasing>(block);
    new(&(m_nextBlock->skipNode)) SkipNode<Increasing>(block);
	block++;

    skipNode = &((m_nextBlock)->skipNode);
    PriQueueNode<Increasing>* pq5 = &((m_nextBlock)->priQueueNode);
    m_nextBlock++;

    new(getCircleEventFromSkipNode(skipNode)) CircleEvent<Increasing>(
		5.0, 0.0, glm::dvec3(0.0, 0.0, 0.0));

    CircleEvent<Increasing>* ce5 = getCircleEventFromSkipNode(skipNode);


    // initialize
    PriQueue<Increasing> pq;
    pq.push(pq1);
    pq.push(pq2);
    pq.push(pq3);
    pq.push(pq4);
    pq.push(pq5);

    // do checks
    EXPECT_EQ(getPriQueueNodeFromCircleEvent(ce1)->prev, -1);
    EXPECT_EQ(NODE(getPriQueueNodeFromCircleEvent(ce1),next), getPriQueueNodeFromCircleEvent(ce2));

    EXPECT_EQ(NODE(getPriQueueNodeFromCircleEvent(ce2),prev), getPriQueueNodeFromCircleEvent(ce1));
    EXPECT_EQ(NODE(getPriQueueNodeFromCircleEvent(ce2),next), getPriQueueNodeFromCircleEvent(ce3));

    EXPECT_EQ(NODE(getPriQueueNodeFromCircleEvent(ce3),prev), getPriQueueNodeFromCircleEvent(ce2));
    EXPECT_EQ(NODE(getPriQueueNodeFromCircleEvent(ce3),next), getPriQueueNodeFromCircleEvent(ce4));

    EXPECT_EQ(NODE(getPriQueueNodeFromCircleEvent(ce4),prev), getPriQueueNodeFromCircleEvent(ce3));
    EXPECT_EQ(NODE(getPriQueueNodeFromCircleEvent(ce4),next), getPriQueueNodeFromCircleEvent(ce5));

    EXPECT_EQ(NODE(getPriQueueNodeFromCircleEvent(ce5),prev), getPriQueueNodeFromCircleEvent(ce4));
    EXPECT_EQ(getPriQueueNodeFromCircleEvent(ce5)->next, -1);

    free (m_memBlocks);
}

// TEST(PriQueueTests, TestInsertOutOfOrderLinkedList)
// {
//     // initialize
//     PriQueue<Increasing> pq;

//     CircleEvent<Increasing> ce3 = CircleEvent<Increasing>(3.0, 0.0, glm::dvec3(0.0, 0.0, 0.0));
//     PriQueueNode<Increasing> pq3 = PriQueueNode<Increasing>(&ce3);
//     pq.push(&pq3);

//     CircleEvent<Increasing> ce1 = CircleEvent<Increasing>(1.0, 0.0, glm::dvec3(0.0, 0.0, 0.0));
//     PriQueueNode<Increasing> pq1 = PriQueueNode<Increasing>(&ce1);
//     pq.push(&pq1);

//     CircleEvent<Increasing> ce5 = CircleEvent<Increasing>(5.0, 0.0, glm::dvec3(0.0, 0.0, 0.0));
//     PriQueueNode<Increasing> pq5 = PriQueueNode<Increasing>(&ce5);
//     pq.push(&pq5);

//     CircleEvent<Increasing> ce4 = CircleEvent<Increasing>(4.0, 0.0, glm::dvec3(0.0, 0.0, 0.0));
//     PriQueueNode<Increasing> pq4 = PriQueueNode<Increasing>(&ce4);
//     pq.push(&pq4);

//     CircleEvent<Increasing> ce2 = CircleEvent<Increasing>(2.0, 0.0, glm::dvec3(0.0, 0.0, 0.0));
//     PriQueueNode<Increasing> pq2 = PriQueueNode<Increasing>(&ce2);
//     pq.push(&pq2);

//     // do checks
//     EXPECT_EQ(ce1.pnode->prev, (PriQueueNode<Increasing>*)NULL);
//     EXPECT_EQ(ce1.pnode->next, ce2.pnode);

//     EXPECT_EQ(ce2.pnode->prev, ce1.pnode);
//     EXPECT_EQ(ce2.pnode->next, ce3.pnode);

//     EXPECT_EQ(ce3.pnode->prev, ce2.pnode);
//     EXPECT_EQ(ce3.pnode->next, ce4.pnode);

//     EXPECT_EQ(ce4.pnode->prev, ce3.pnode);
//     EXPECT_EQ(ce4.pnode->next, ce5.pnode);

//     EXPECT_EQ(ce5.pnode->prev, ce4.pnode);
//     EXPECT_EQ(ce5.pnode->next, (PriQueueNode<Increasing>*)NULL);
// }

// TEST(PriQueueTests, TestInsertReverseOrderLinkedList)
// {
//     // initialize
//     PriQueue<Increasing> pq;

//     CircleEvent<Increasing> ce5 = CircleEvent<Increasing>(5.0, 0.0, glm::dvec3(0.0, 0.0, 0.0));
//     PriQueueNode<Increasing> pq5 = PriQueueNode<Increasing>(&ce5);
//     pq.push(&pq5);

//     CircleEvent<Increasing> ce4 = CircleEvent<Increasing>(4.0, 0.0, glm::dvec3(0.0, 0.0, 0.0));
//     PriQueueNode<Increasing> pq4 = PriQueueNode<Increasing>(&ce4);
//     pq.push(&pq4);

//     CircleEvent<Increasing> ce3 = CircleEvent<Increasing>(3.0, 0.0, glm::dvec3(0.0, 0.0, 0.0));
//     PriQueueNode<Increasing> pq3 = PriQueueNode<Increasing>(&ce3);
//     pq.push(&pq3);

//     CircleEvent<Increasing> ce2 = CircleEvent<Increasing>(2.0, 0.0, glm::dvec3(0.0, 0.0, 0.0));
//     PriQueueNode<Increasing> pq2 = PriQueueNode<Increasing>(&ce2);
//     pq.push(&pq2);

//     CircleEvent<Increasing> ce1 = CircleEvent<Increasing>(1.0, 0.0, glm::dvec3(0.0, 0.0, 0.0));
//     PriQueueNode<Increasing> pq1 = PriQueueNode<Increasing>(&ce1);
//     pq.push(&pq1);

//     // do checks
//     EXPECT_EQ(ce1.pnode->prev, (PriQueueNode<Increasing>*)NULL);
//     EXPECT_EQ(ce1.pnode->next, ce2.pnode);

//     EXPECT_EQ(ce2.pnode->prev, ce1.pnode);
//     EXPECT_EQ(ce2.pnode->next, ce3.pnode);

//     EXPECT_EQ(ce3.pnode->prev, ce2.pnode);
//     EXPECT_EQ(ce3.pnode->next, ce4.pnode);

//     EXPECT_EQ(ce4.pnode->prev, ce3.pnode);
//     EXPECT_EQ(ce4.pnode->next, ce5.pnode);

//     EXPECT_EQ(ce5.pnode->prev, ce4.pnode);
//     EXPECT_EQ(ce5.pnode->next, (PriQueueNode<Increasing>*)NULL);
// }

// TEST(PriQueueTests, TestInsertSkips)
// {
//     // initialize
//     PriQueue<Increasing> pq;

//     CircleEvent<Increasing> ce3 = CircleEvent<Increasing>(3.0, 0.0, glm::dvec3(0.0, 0.0, 0.0));
//     PriQueueNode<Increasing> pq3 = PriQueueNode<Increasing>(&ce3);
//     pq.push(&pq3);

//     CircleEvent<Increasing> ce1 = CircleEvent<Increasing>(1.0, 0.0, glm::dvec3(0.0, 0.0, 0.0));
//     PriQueueNode<Increasing> pq1 = PriQueueNode<Increasing>(&ce1);
//     pq.push(&pq1);

//     CircleEvent<Increasing> ce5 = CircleEvent<Increasing>(5.0, 0.0, glm::dvec3(0.0, 0.0, 0.0));
//     PriQueueNode<Increasing> pq5 = PriQueueNode<Increasing>(&ce5);
//     pq.push(&pq5);

//     CircleEvent<Increasing> ce4 = CircleEvent<Increasing>(4.0, 0.0, glm::dvec3(0.0, 0.0, 0.0));
//     PriQueueNode<Increasing> pq4 = PriQueueNode<Increasing>(&ce4);
//     pq.push(&pq4);

//     CircleEvent<Increasing> ce2 = CircleEvent<Increasing>(2.0, 0.0, glm::dvec3(0.0, 0.0, 0.0));
//     PriQueueNode<Increasing> pq2 = PriQueueNode<Increasing>(&ce2);
//     pq.push(&pq2);

//     // do checks
//     PriQueueNode<Increasing>* curr = pq.head;

//     while (curr != NULL)
//     {
//         for (int i = 0; i < 12; i++)
//         {
//             if (curr->skips[i] != NULL) EXPECT_EQ(curr, curr->skips[i]->prev_skips[i]);
//             if (curr->next != NULL) EXPECT_EQ(curr, curr->next->prev);
//         }

//         curr = curr->next;
//     }
// }

// TEST(PriQueueTests, TestEraseSkips)
// {
//     // initialize
//     PriQueue<Increasing> pq;

//     CircleEvent<Increasing> ce3 = CircleEvent<Increasing>(3.0, 0.0, glm::dvec3(0.0, 0.0, 0.0));
//     PriQueueNode<Increasing> pq3 = PriQueueNode<Increasing>(&ce3);
//     pq.push(&pq3);

//     CircleEvent<Increasing> ce1 = CircleEvent<Increasing>(1.0, 0.0, glm::dvec3(0.0, 0.0, 0.0));
//     PriQueueNode<Increasing> pq1 = PriQueueNode<Increasing>(&ce1);
//     pq.push(&pq1);

//     CircleEvent<Increasing> ce5 = CircleEvent<Increasing>(5.0, 0.0, glm::dvec3(0.0, 0.0, 0.0));
//     PriQueueNode<Increasing> pq5 = PriQueueNode<Increasing>(&ce5);
//     pq.push(&pq5);

//     CircleEvent<Increasing> ce4 = CircleEvent<Increasing>(4.0, 0.0, glm::dvec3(0.0, 0.0, 0.0));
//     PriQueueNode<Increasing> pq4 = PriQueueNode<Increasing>(&ce4);
//     pq.push(&pq4);

//     CircleEvent<Increasing> ce2 = CircleEvent<Increasing>(2.0, 0.0, glm::dvec3(0.0, 0.0, 0.0));
//     PriQueueNode<Increasing> pq2 = PriQueueNode<Increasing>(&ce2);
//     pq.push(&pq2);

//     pq.erase(ce3.pnode);

//     // do checks
//     PriQueueNode<Increasing>* curr = pq.head;

//     while (curr != NULL)
//     {
//         for (int i = 0; i < 12; i++)
//         {
//             if (curr->skips[i] != NULL) EXPECT_EQ(curr, curr->skips[i]->prev_skips[i]);
//             if (curr->next != NULL) EXPECT_EQ(curr, curr->next->prev);
//         }

//         curr = curr->next;
//     }
// }

// TEST(PriQueueTests, TestEraseHeadSkips)
// {
//     // initialize
//     PriQueue<Increasing> pq;

//     CircleEvent<Increasing> ce3 = CircleEvent<Increasing>(3.0, 0.0, glm::dvec3(0.0, 0.0, 0.0));
//     PriQueueNode<Increasing> pq3 = PriQueueNode<Increasing>(&ce3);
//     pq.push(&pq3);

//     CircleEvent<Increasing> ce1 = CircleEvent<Increasing>(1.0, 0.0, glm::dvec3(0.0, 0.0, 0.0));
//     PriQueueNode<Increasing> pq1 = PriQueueNode<Increasing>(&ce1);
//     pq.push(&pq1);

//     CircleEvent<Increasing> ce5 = CircleEvent<Increasing>(5.0, 0.0, glm::dvec3(0.0, 0.0, 0.0));
//     PriQueueNode<Increasing> pq5 = PriQueueNode<Increasing>(&ce5);
//     pq.push(&pq5);

//     CircleEvent<Increasing> ce4 = CircleEvent<Increasing>(4.0, 0.0, glm::dvec3(0.0, 0.0, 0.0));
//     PriQueueNode<Increasing> pq4 = PriQueueNode<Increasing>(&ce4);
//     pq.push(&pq4);

//     CircleEvent<Increasing> ce2 = CircleEvent<Increasing>(2.0, 0.0, glm::dvec3(0.0, 0.0, 0.0));
//     PriQueueNode<Increasing> pq2 = PriQueueNode<Increasing>(&ce2);
//     pq.push(&pq2);

//     pq.erase(ce1.pnode);

//     // do checks
//     PriQueueNode<Increasing>* curr = pq.head;

//     while (curr != NULL)
//     {
//         for (int i = 0; i < 12; i++)
//         {
//             if (curr->skips[i] != NULL) EXPECT_EQ(curr, curr->skips[i]->prev_skips[i]);
//             if (curr->next != NULL) EXPECT_EQ(curr, curr->next->prev);
//         }

//         curr = curr->next;
//     }
// }

// TEST(PriQueueTests, TestEraseTailSkips)
// {
//     // initialize
//     PriQueue<Increasing> pq;

//     CircleEvent<Increasing> ce3 = CircleEvent<Increasing>(3.0, 0.0, glm::dvec3(0.0, 0.0, 0.0));
//     PriQueueNode<Increasing> pq3 = PriQueueNode<Increasing>(&ce3);
//     pq.push(&pq3);

//     CircleEvent<Increasing> ce1 = CircleEvent<Increasing>(1.0, 0.0, glm::dvec3(0.0, 0.0, 0.0));
//     PriQueueNode<Increasing> pq1 = PriQueueNode<Increasing>(&ce1);
//     pq.push(&pq1);

//     CircleEvent<Increasing> ce5 = CircleEvent<Increasing>(5.0, 0.0, glm::dvec3(0.0, 0.0, 0.0));
//     PriQueueNode<Increasing> pq5 = PriQueueNode<Increasing>(&ce5);
//     pq.push(&pq5);

//     CircleEvent<Increasing> ce4 = CircleEvent<Increasing>(4.0, 0.0, glm::dvec3(0.0, 0.0, 0.0));
//     PriQueueNode<Increasing> pq4 = PriQueueNode<Increasing>(&ce4);
//     pq.push(&pq4);

//     CircleEvent<Increasing> ce2 = CircleEvent<Increasing>(2.0, 0.0, glm::dvec3(0.0, 0.0, 0.0));
//     PriQueueNode<Increasing> pq2 = PriQueueNode<Increasing>(&ce2);
//     pq.push(&pq2);

//     pq.erase(ce5.pnode);

//     // do checks
//     PriQueueNode<Increasing>* curr = pq.head;

//     while (curr != NULL)
//     {
//         for (int i = 0; i < 12; i++)
//         {
//             if (curr->skips[i] != NULL) EXPECT_EQ(curr, curr->skips[i]->prev_skips[i]);
//             if (curr->next != NULL) EXPECT_EQ(curr, curr->next->prev);
//         }

//         curr = curr->next;
//     }
// }

// TEST(PriQueueTests, TestErasePopPushSkips)
// {
//     // initialize
//     PriQueue<Increasing> pq;

//     CircleEvent<Increasing> ce3 = CircleEvent<Increasing>(3.0, 0.0, glm::dvec3(0.0, 0.0, 0.0));
//     PriQueueNode<Increasing> pq3 = PriQueueNode<Increasing>(&ce3);
//     pq.push(&pq3);

//     CircleEvent<Increasing> ce1 = CircleEvent<Increasing>(1.0, 0.0, glm::dvec3(0.0, 0.0, 0.0));
//     PriQueueNode<Increasing> pq1 = PriQueueNode<Increasing>(&ce1);
//     pq.push(&pq1);

//     CircleEvent<Increasing> ce5 = CircleEvent<Increasing>(5.0, 0.0, glm::dvec3(0.0, 0.0, 0.0));
//     PriQueueNode<Increasing> pq5 = PriQueueNode<Increasing>(&ce5);
//     pq.push(&pq5);

//     CircleEvent<Increasing> ce4 = CircleEvent<Increasing>(4.0, 0.0, glm::dvec3(0.0, 0.0, 0.0));
//     PriQueueNode<Increasing> pq4 = PriQueueNode<Increasing>(&ce4);
//     pq.push(&pq4);

//     CircleEvent<Increasing> ce2 = CircleEvent<Increasing>(2.0, 0.0, glm::dvec3(0.0, 0.0, 0.0));
//     PriQueueNode<Increasing> pq2 = PriQueueNode<Increasing>(&ce2);
//     pq.push(&pq2);

//     pq.erase(ce5.pnode);
//     pq.pop();
//     pq.erase(ce3.pnode);

//     CircleEvent<Increasing> ce6 = CircleEvent<Increasing>(2.5, 0.0, glm::dvec3(0.0, 0.0, 0.0));
//     PriQueueNode<Increasing> pq6 = PriQueueNode<Increasing>(&ce6);
//     pq.push(&pq6);

//     CircleEvent<Increasing> ce7 = CircleEvent<Increasing>(7.0, 0.0, glm::dvec3(0.0, 0.0, 0.0));
//     PriQueueNode<Increasing> pq7 = PriQueueNode<Increasing>(&ce7);
//     pq.push(&pq7);

//     pq.pop();
//     pq.pop();
//     pq.pop();

//     CircleEvent<Increasing> ce8 = CircleEvent<Increasing>(1.1, 0.0, glm::dvec3(0.0, 0.0, 0.0));
//     PriQueueNode<Increasing> pq8 = PriQueueNode<Increasing>(&ce8);
//     pq.push(&pq8);

//     // do checks
//     PriQueueNode<Increasing>* curr = pq.head;

//     while (curr != NULL)
//     {
//         for (int i = 0; i < 12; i++)
//         {
//             if (curr->skips[i] != NULL) EXPECT_EQ(curr, curr->skips[i]->prev_skips[i]);
//             if (curr->next != NULL) EXPECT_EQ(curr, curr->next->prev);
//         }

//         curr = curr->next;
//     }
// }