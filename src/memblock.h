#ifndef MEMBLOCK_H
#define MEMBLOCK_H

#include "beachline.h"
#include "voronoi_event.h"

#include "platform.h"

template <Order O>
struct MemBlock
{
    SkipNode<O> skipNode;
    CircleEvent<O> circleEvent;
};

template struct MemBlock<Increasing>;
template struct MemBlock<Decreasing>;

template <Order O>
const int skipNodeOffset = OFFSETOF(MemBlock<O>, skipNode);

template <Order O>
const int circleEventOffset = OFFSETOF(MemBlock<O>, circleEvent);

template <Order O>
const int beachArcOffset = OFFSETOF(MemBlock<O>, skipNode.m_beachArc);

template <Order O>
const int ceTOsn = skipNodeOffset<O> - circleEventOffset<O>;

template <Order O>
inline SkipNode<O>* getSkipNodeFromCircleEvent(CircleEvent<O>* circleEvent)
{
    return (SkipNode<O>*)((char*)circleEvent + ceTOsn<O>);
}

template <Order O>
const int snTOce = circleEventOffset<O> - skipNodeOffset<O>;

template <Order O>
inline CircleEvent<O>* getCircleEventFromSkipNode(SkipNode<O>* skipNode)
{
    return (CircleEvent<O>*)((char*)skipNode + snTOce<O>);
}

template <Order O>
inline SkipNode<O>* getPointerFromIndex(SkipNode<O>* skipNode, int i)
{
    return (SkipNode<O>*)( (char*)skipNode + (i - skipNode->index) * sizeof(MemBlock<O>) );
}

#define NODE(pointer, member) getPointerFromIndex(pointer, pointer->member)
#define NODE_2(pointer, member) getPointerFromIndex(pointer, getPointerFromIndex(pointer, pointer->member)->member)

#endif