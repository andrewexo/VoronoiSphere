#include "beachline.h"
#include "memblock.h"
#include "platform.h"
#include <cstring>
#include <algorithm>
#include <iostream>

namespace VorGen {

#define _USE_MATH_DEFINES
#include <math.h>

// SSE2
#include <emmintrin.h>
#include <smmintrin.h>

constexpr int SKIP_DEPTH_B_sub1 = SKIP_DEPTH_B - 1;

// SkipNode Implementation //

template <Order O>
inline void SkipNode<O>::init(int i)
{
    index = i;
    memset(skips, -1, sizeof(int) * SKIP_DEPTH_B);
    memset(p_skips, -1, sizeof(int) * SKIP_DEPTH_B);
    prev = next = -1;
    range_end = 0.0;
}

template <Order O>
inline void SkipNode<O>::initSite(VoronoiSite* site, uint8_t threadId)
{
    m_beachArc.m_site = site;
    site->m_cell->increment(threadId);
}

template <>
SkipNode<Increasing>::SkipNode(int i)
{
    sweepline_pos = 0.0;

    init(i);
}

template <>
SkipNode<Decreasing>::SkipNode(int i)
{
    sweepline_pos = M_PI;

    init(i);
}

template <Order O>
SkipNode<O>::~SkipNode()
{
    
}

template <>
double SkipNode<Increasing>
::getRangeEnd(const SweepLine & sl,
              double shift,
              SkipNode<Increasing>* other)
{
    if (sl.m_polar > sweepline_pos)
    {
        ALIGN(16) double ends[2];
        intersect2(m_beachArc.m_site,
                   NODE(this,next)->m_beachArc.m_site,
                   other->m_beachArc.m_site,
                   NODE(other,next)->m_beachArc.m_site,
                   sl,
                   shift,
                   ends);

        range_end = ends[1];
        other->range_end = ends[0];

        sweepline_pos = sl.m_polar;
        other->sweepline_pos = sl.m_polar;
    }

    return range_end;
}

template <>
double SkipNode<Decreasing>
::getRangeEnd(const SweepLine & sl,
              double shift,
              SkipNode<Decreasing>* other)
{
    if (sl.m_polar < sweepline_pos)
    {
        ALIGN(16) double ends[2];
        intersect2(NODE(other,next)->m_beachArc.m_site,
                        other->m_beachArc.m_site,
                        NODE(this,next)->m_beachArc.m_site,
                        m_beachArc.m_site,
                        sl,
                        shift,
                        ends);

        range_end = ends[0];
        other->range_end = ends[1];

        sweepline_pos = sl.m_polar;
        other->sweepline_pos = sl.m_polar;
    }

    return range_end;
}


constexpr double PI2 = M_PI*2.0;
constexpr double PI2i = 0.5/M_PI;

template <Order O>
double SkipNode<O>::intersect(VoronoiSite* siteA, VoronoiSite* siteB, 
                              const SweepLine & sl, double shift)
{
    double eps = (siteA->m_polCos - siteB->m_polCos) * sl.m_polSin;

    double a = (sl.m_polCos - siteB->m_polCos) * siteA->m_aziCosPS -
                (sl.m_polCos - siteA->m_polCos) * siteB->m_aziCosPS;

    double b = (sl.m_polCos - siteB->m_polCos) * siteA->m_aziSinPS -
                (sl.m_polCos - siteA->m_polCos) * siteB->m_aziSinPS;

    double ab_hyp = 1.0L / sqrt(a*a + b*b);

    double gamma;
    if (b > 0) gamma = asin(a*ab_hyp);
    else
    {
        if (a > 0) gamma = acos(b*ab_hyp);
        else gamma = acos(b*ab_hyp) - 2*asin(a*ab_hyp);
    }

    double azi = asin(eps * ab_hyp) - gamma;

    azi += shift;
    azi = azi * PI2i;
    azi = azi - floor(azi);

    return azi * PI2;
}

// compiler replaces this with sse2_asin
inline void asin128(__m128d & inout)
{
    ALIGN(16) double d[2];
    _mm_store_pd(d, inout);

    d[0] = asin(d[0]);
    d[1] = asin(d[1]);

    inout = _mm_load_pd(d);
}

// compiler replaces this with sse2_acos
inline void acos128(__m128d & inout)
{
    ALIGN(16) double d[2];
    _mm_store_pd(d, inout);

    d[0] = acos(d[0]);
    d[1] = acos(d[1]);

    inout = _mm_load_pd(d);
}

template <Order O>
void SkipNode<O>::intersect2(VoronoiSite* siteA, VoronoiSite* siteB, 
                             VoronoiSite* siteC, VoronoiSite* siteD, 
                             const SweepLine & sl, double shift, double* out)
{
    ALIGN(16) double zero[2] = { 0.0,0.0 }; __m128d* ZERO = (__m128d*)zero;
    ALIGN(16) double one[2]  = { 1.0,1.0 }; __m128d* ONE  = (__m128d*)one;
    ALIGN(16) double two[2]  = { 2.0,2.0 }; __m128d* TWO  = (__m128d*)two;

    __m128d EE = _mm_set1_pd(sl.m_polCos);

    __m128d FM = _mm_set_pd(siteB->m_polCos, siteD->m_polCos);
    __m128d GN = _mm_set_pd(siteA->m_polCos, siteC->m_polCos);

    __m128d W = _mm_sub_pd(EE, FM);
    __m128d Z = _mm_sub_pd(EE, GN);

    __m128d HO = _mm_set_pd(siteA->m_aziCosPS, siteC->m_aziCosPS);
    __m128d IP = _mm_set_pd(siteB->m_aziCosPS, siteD->m_aziCosPS);
    __m128d JQ = _mm_set_pd(siteA->m_aziSinPS, siteC->m_aziSinPS);
    __m128d KR = _mm_set_pd(siteB->m_aziSinPS, siteD->m_aziSinPS);

    __m128d X = _mm_mul_pd(W, HO);
    __m128d S = _mm_mul_pd(Z, IP);
    __m128d Y = _mm_mul_pd(W, JQ);
    __m128d T = _mm_mul_pd(Z, KR);

    __m128d AC = _mm_sub_pd(X, S);
    __m128d BD = _mm_sub_pd(Y, T);

    __m128d U = _mm_mul_pd(AC, AC);
    __m128d V = _mm_mul_pd(BD, BD);

    __m128d BETA = _mm_add_pd(U, V);
    BETA = _mm_sqrt_pd(BETA);

    __m128d ALPHA = _mm_div_pd(*ONE, BETA);

    __m128d A_ALPHA = _mm_mul_pd(AC, ALPHA);
    __m128d B_ALPHA = _mm_mul_pd(BD, ALPHA);

    asin128(A_ALPHA);
    acos128(B_ALPHA);

    __m128d C_ALPHA = _mm_mul_pd(*TWO, A_ALPHA);
    C_ALPHA = _mm_sub_pd(B_ALPHA, C_ALPHA);

    __m128d LTZ = _mm_cmple_pd(BD, *ZERO);
    __m128d GTZ = _mm_cmpgt_pd(AC, *ZERO);

    __m128d GAMMA = _mm_blendv_pd(C_ALPHA, B_ALPHA, GTZ);
    GAMMA = _mm_blendv_pd(A_ALPHA, GAMMA, LTZ);

    EE = _mm_set1_pd(sl.m_polSin);
    __m128d EPS = _mm_sub_pd(GN, FM);
    EE = _mm_mul_pd(EE, ALPHA);
    EPS = _mm_mul_pd(EPS, EE);

    asin128(EPS);
    EPS = _mm_sub_pd(EPS, GAMMA);

    __m128d SHIFT = _mm_set1_pd(shift);
    __m128d PI2I_ = _mm_set1_pd(PI2i);
    __m128d PI2_ = _mm_set1_pd(PI2);

    EPS = _mm_add_pd(EPS, SHIFT);
    EPS = _mm_mul_pd(EPS, PI2I_);
    EPS = _mm_sub_pd(EPS, _mm_floor_pd(EPS));
    EPS = _mm_mul_pd(EPS, PI2_);

    _mm_store_pd(out, EPS);
}


// BeachLine Implementation //

constexpr int DIST_MAX = 1 << (SKIP_DEPTH_B + 1);

template <Order O>
BeachLine<O>::BeachLine()
{
    size = 0;
    linked_list = NULL;
    distribution = ::std::uniform_int_distribution<int>(0,DIST_MAX);
}

template <Order O>
BeachLine<O>::~BeachLine()
{
}

template <Order O>
int BeachLine<O>::getSize()
{
    return size;
}

template <Order O>
void BeachLine<O>::insert1(SkipNode<O>* node)
{
    insertAfter(node, NULL);
}

template <Order O>
void BeachLine<O>::insert2(SkipNode<O>* node)
{
    insertAfter(node, linked_list);
    addSkips(node, &linked_list, true);
}

template <Order O>
void BeachLine<O>::erase(SkipNode<O>* node, uint8_t threadId)
{
    // change starting position for searches if necessary
    if (node == linked_list)
    {
        if (linked_list->skips[SKIP_DEPTH_B_sub1] != linked_list->index)
        {
            linked_list = NODE(linked_list, skips[SKIP_DEPTH_B_sub1]);
        }
        else
        {
            SkipNode<O>* next = NODE(linked_list, next);

            for (int i = 0; i < SKIP_DEPTH_B; i++)
            {	
                if (next->skips[i] == -1)
                {
                    next->skips[i] = linked_list->skips[i];
                    NODE(linked_list, skips[i])->p_skips[i] = linked_list->next;
                    next->p_skips[i] = linked_list->index;
                    linked_list->skips[i] = linked_list->next;
                }
            }

            linked_list = next;
        }
    }

    removeSkips(node);

    // erase from list
    NODE(node, prev)->next = node->next;
    NODE(node, next)->prev = node->prev;

    node->m_beachArc.m_site->m_cell->decrement(threadId);

    size--;
}

template <Order O>
void BeachLine<O>::removeSkips(SkipNode<O>* node)
{
    for (int i = 0; i < SKIP_DEPTH_B; i++)
    {
        if (node->skips[i] == -1) break;

        NODE(node, skips[i])->p_skips[i] = node->p_skips[i];
        NODE(node, p_skips[i])->skips[i] = node->skips[i];
    }
}

template <Order O>
bool BeachLine<O>::isRangeEndGreater(SkipNode<O>* next, SkipNode<O>* curr, SweepLine & sl, double shift, int skipLevel)
{
    double c = curr->getRangeEnd(sl, shift, next);
    return (next->getRangeEnd(sl, shift, NODE(next, skips[skipLevel])) > c);
}

// 3.75% - 6.72%
template <Order O>
void BeachLine<O>::findAndInsert(SkipNode<O>* node, SkipNode<O>* node2, double sweepline, uint8_t threadId)
{
    // shift positions on beachline such that the new insertion point goes to zero
    // this means we want to search for the element with the largest post intersection value

    double shift = 2.0 * M_PI - node->m_beachArc.m_site->m_azimuth;

    int skip_level = SKIP_DEPTH_B_sub1;
    SkipNode<O>* nodes[SKIP_DEPTH_B];
    SkipNode<O>* curr = linked_list;

    SweepLine sl;
    sl.m_polar = sweepline;
    sl.m_polCos = cos(sweepline);
    sl.m_polSin = sin(sweepline);

    while (true)
    {
        if (isRangeEndGreater(NODE(curr, skips[skip_level]), curr, sl, shift, skip_level))
        {
            curr = NODE(curr, skips[skip_level]);
        }
        else
        {
            nodes[skip_level--] = curr;
            if (skip_level < 0) break;
        }
    }

    // continue search on the linked list level
    double currRangeEnd = curr->getRangeEnd(sl, shift, NODE(curr, next));
    double currRangeEndNext;
    while ( (currRangeEndNext = NODE(curr, next)->getRangeEnd(sl, shift, NODE_2(curr, next))) > currRangeEnd )
    {
        curr = NODE(curr, next);
        currRangeEnd = currRangeEndNext;
    }

    curr = NODE(curr, next);

    // split node and insert in between
    node2->initSite(curr->m_beachArc.m_site, threadId);

    insertAfter(node, curr);
    insertAfter(node2, node);

    addSkips(node,nodes,false);
    addSkips(node2,nodes,false);
}

template <Order O>
void BeachLine<O>::insertAfter(SkipNode<O>* node, SkipNode<O>* at)
{
    if (at == NULL)
    {
        node->next = node->index;
        node->prev = node->index;
        linked_list = node;

        for (int i = 0; i < SKIP_DEPTH_B; i++)
        {
            node->skips[i] = node->index;
            node->p_skips[i] = node->index;
        }
    }
    else
    {
        // insert into list
        SkipNode<O>* next = NODE(at, next);
        at->next = node->index;
        node->prev = at->index;
        node->next = next->index;
        next->prev = node->index;
    }

    size++;
}

inline int log2(int n)
{
    const unsigned int b[] = {0x2, 0xC, 0xF0, 0xFF00, 0xFFFF0000};
    const unsigned int S[] = {1, 2, 4, 8, 16};

    unsigned int r = 0; // result of log2(v) will go here
    for (int i = 3; i >= 0; i--) // unroll for speed (if compiler doesn't already)
    {
      if (n & b[i])
      {
        n >>= S[i];
        r |= S[i];
      } 
    }

    return r;
}

template <Order O>
void BeachLine<O>::addSkips(SkipNode<O>* node, SkipNode<O>** previous, bool repeat_first)
{
    int skip_count = SKIP_DEPTH_B - log2(::std::max((int)(distribution(generator)), 1));

    for(int i = 0; i < skip_count; i++)
    {
        node->skips[i] = (*previous)->skips[i];
        NODE(node, skips[i])->p_skips[i] = node->index;
        (*previous)->skips[i] = node->index;
        node->p_skips[i] = (*previous)->index;

        if (!repeat_first)
        {
            (*previous) = node;
            previous++;
        }
    }
}

template class SkipNode<Increasing>;
template class SkipNode<Decreasing>;

template class BeachLine<Increasing>;
template class BeachLine<Decreasing>;

}
